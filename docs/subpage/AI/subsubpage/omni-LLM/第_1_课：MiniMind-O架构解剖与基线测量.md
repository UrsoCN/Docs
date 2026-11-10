---
created: 2026-11-09
modified: 2026-11-10
tags: [type/learning, topic/speech, topic/llm, topic/audio]
---

# 第 1 课：MiniMind-O 架构解剖与基线测量

> 本卷的[总纲与路线图](./端到端语音大模型学习笔记.md)在这里。
> LLM 基础前置：[第 0 课：从音频工程到语言模型](第_0_课：LLM基础_从音频工程到语言模型.md)。
> 前置：[《智能语音》第 8 课 LLM-based ASR](../智能语音/第_8_课：Whisper与LLM-based-ASR.md)、[第 11 课 TTS](../智能语音/第_11_课：神经语音合成-TTS.md)、[第 17 课 端到端延迟优化](../智能语音/智能语音服务学习笔记.md)。
> 状态：🔄 进行中（实验待在本机/Orin NX 上补齐数字）

---

## 0. 本课要回答的三个问题

1. 一次完整推理里，数据在每一步长什么样？（**架构解剖**）
2. 延迟花在哪？首包能不能撑住机器人交互？（**基线测量**）
3. 它的"语速"是谁决定的？（**码率算术**）

这三个问题不回答，后面所有"优化"都是盲改。

---

## 1. 环境：先认清手里有什么

### 1.1 本机现状（2026-11-09 实测）

> 历史快照：以下表格记录 11 月 9 日状态。11 月 10 日用户已完成环境配置并试运行模型；不应再据此判断当前缺少依赖或权重。后续性能实验需记录当次环境。

| 项 | 状态 |
|---|---|
| `nvidia-smi` | 不存在 |
| `torch` | 2.13.0+cu130，但 `torch.cuda.is_available() == False` |
| `transformers` | **未安装**（`pip install -r requirements.txt` 还没跑） |
| `model/` 下的编码器 | 未下载（只有 `model/*.py` 和 tokenizer） |
| `out/` 权重 | 未下载 |

所以第一阶段的三条路要分清：

| 用途 | 在哪跑 | 说明 |
|---|---|---|
| 代码走读 + CPU 推理 | 本机 | 0.1B + 425M 冻结，CPU 能出结果，只是慢；**用来验证正确性，不用来测性能** |
| 训练（mini 全链 2h） | 训练服务器 | 3090 级单卡约 2h 跑通；full 数据另算。别在 Orin NX 上训 |
| 部署验证（RTF/内存） | Orin NX | 538M runtime 是它相对 Qwen3-Omni 的核心优势，要尽早验证 |

### 1.2 依赖与资产

```bash
cd ~/source/open_source/minimind-o
pip install -r requirements.txt
bash bench/fetch_assets.sh          # SenseVoice + Mimi + CAM++（+ SigLIP2）
bash bench/fetch_assets.sh --full   # 额外拉 sft_omni_768.pth
```

| 资产 | 作用 | 是否必需 |
|---|---|---|
| `model/SenseVoiceSmall` | 音频编码器（16k fbank → 512 维，冻结 234M） | 跑 A2A 必需 |
| `model/mimi` | 语音编解码器（24k，12.5Hz，8 codebook，冻结 96M） | 出音频必需 |
| `model/campplus` | 说话人嵌入（192 维） | 音色克隆必需 |
| `model/siglip2-base-p32-256-ve` | 视觉编码器（冻结 95M） | 只跑语音可跳过 |
| `out/sft_omni_768.pth` | 发布权重 | 必需 |

> 注意 `eval_omni.py` 默认 `--load_from model`，它读的是 `./out/sft_omni_768.pth`；`--load_from <hf目录>` 才走 transformers 路径。两条路的 dtype 和视觉加载行为不同，第 6 节会踩到这个坑。

---

## 2. 架构解剖

### 2.1 一条样本长什么样

`dataset/omni_dataset.py` 把一条训练样本拼成 **9 路序列**（8 路 audio + 1 路 text），长度对齐到 `max_length`：

```
        ┌──────────── prompt（文本 token，含 <|audio_pad|> / <|image_pad|> 占位）────────────┐
text:   [ system ][ user ][ audio_pad × L ][ question ][ assistant ][ 回答 token … ][ eos ][ pad ]
                                                                └──── 只有这里算 text loss ────┘

audio:  [ pad ……………………………………………………… spk ][ ref_codes ][ 第0层 code … ][ stop ][ pad ]
        [ pad …………………………………………………………………………………… ][ 第1层 code（晚 1 步）… ]
        …                                                        └ 8 层各自晚 i 步 ┘
```

关键规则（都在 `omni_dataset.py`）：

1. 音频目标从 `assistant_start + layer_idx + 1` 开始，**每层比文本晚 i 步**。
2. 只有最后一个 assistant 段算 loss；前面轮次和多模态占位符位置都是 `-100`。
3. 参考音色（`ref_codes`）右对齐塞在 assistant 起始位置之前，`spk_emb` 占一个位置——它们只做条件，不做重构目标。

### 2.2 前向：Thinker 与 Talker 是两条独立通路

`model_omni.py::forward()` 的核心就 20 行：

```python
hidden_states = thinker.embed_tokens(text_ids)          # 只看文本 token
if audio_inputs:  hidden_states = inject_audio_features(...)   # 音频特征替换 audio_pad 位置
for i, layer in enumerate(thinker.layers):
    hidden_states = layer(...)
    if i == config.bridge_layer: bridge_states = hidden_states  # 默认 8//2-1 = 3
h_thinker = thinker.norm(hidden_states)

talker_emb = talker.embed_tokens(audio_ids)             # 8 路 code 的共享嵌入
hidden = talker.embed_proj(bridge_states) * text_scale + talker.codec_proj(talker_emb) * audio_scale
for layer in talker.layers:  hidden = layer(hidden, ...)
audio_logits = talker.lm_head(talker.norm(hidden))      # 8 个头，各 2112 维
```

三个值得注意的设计：

| 设计 | 为什么这么做 | 代价 |
|---|---|---|
| Bridge 取**中间层**（第 3 层） | embedding 语义不足，最后一层被 LM head 过度塑形；中间层既有上下文又没被任务头带偏 | 换个 Thinker 深度就得重扫 `bridge_layer` |
| Talker 是**独立** 4 层，不与 Thinker 共享 | 声学建模和语义建模的分布差太远，共享会互相拖累 | 多 47M 参数 |
| 8 层 codebook 用**共享 base + rank-256 adapter** | 每层一个完整 head 会爆参数 | 层间分布差异只能靠低秩补偿 |

### 2.3 参数量（README 口径）

| 模块 | 参数 | 状态 |
|---|---:|---|
| Thinker（8 层 / 768） | 63.91M | 可训 |
| Talker（4 层 / 768 / 8 头） | 47.05M | 可训 |
| Audio projector（512→768） | 0.99M | 可训 |
| Vision projector（768→768） | 1.18M | 可训 |
| SenseVoice-Small | 234.00M | **冻结** |
| Mimi | 96.15M | **冻结** |
| SigLIP2 base | 94.55M | **冻结** |
| **运行时合计** | **537.83M** | — |

> 记住这张表：后面所有「能不能上 Orin NX」的问题，分母都是 537.83M，不是 113M。

---

## 3. 码率算术：音频时长由有效音频帧数确定

### 3.1 区分三种长度

```text
文本长度：有效文本 token 数
音频长度：有效音频帧数，每帧包含 8 个码本索引
生成长度：循环推进的位置数，还可能包含文本结束后的填充与音频尾部

音频时长 ≈ 有效音频帧数 / 12.5 秒
```

训练目标从 `assistant_start + layer_idx + 1` 起填充，只说明码本的延迟布局。音频目标有自己的长度；`stream_generate()` 在文本 EOS 后仍可继续生成音频，直到音频停止条件满足。因此不能推导“音频时长 = 文本 token 数 × 80ms”。

### 3.2 Tokenizer 不提供固定发音时长

分词器使用 6400 词表的 byte-level BPE，字节可以合并。当前本地 tokenizer 将“你好”和“机器人”各编码成一个 token。UTF-8 字节数、字符数、token 数和发音时长不能直接等同。

旧稿中的“固定约 4 字/秒”“没有 duration 自由度”和“换词表必然改变到指定语速”均撤回。自然度、停顿和漏读问题需要结合训练数据、声学目标及生成行为分析，不能用这条错误推导解释。

### 3.3 该记录什么

- 分别记录 `n_text_tokens`、`n_audio_frames`、`text_end_ms` 与 `audio_end_ms`，观察文本结束后音频是否仍在继续。
- `chars_per_sec` 描述去空白字符数与音频时长的比值，不预设应该接近 4；数字、标点与英文词也会影响解释。
- `tokens_per_char` 描述当前切分效率；若计数包含 EOS 等特殊 token，应与纯文本分词统计区分。
- 结合解码后的实际时长和试听定位异常，避免只看 token 比例就断言模型的韵律能力。

---

## 4. 基线协议

### 4.1 测什么

| 指标 | 定义 | 机器人上为什么重要 |
|---|---|---|
| `ttft_text_ms` | 从生成循环开始到首个文本 token | 文本侧首响应；完整工具调用还要等参数等内容生成 |
| `ttfa_ms` | 首个**完整 8 层 codec frame** | 尚未包含 PCM 解码、排队和实际发声 |
| `rtf_wall` | 当前 bench 的生成循环耗时 / 音频时长 | 分子不含随后单独执行的 Mimi 解码；不能单独证明整条链路能实时播放 |
| `chars_per_sec` | 去空白字符数 / 音频时长 | 描述输出节奏，配合试听与帧数分析 |
| `decode_ms` | 单独记录的 Mimi 解码调用耗时 | GPU 计时需明确同步边界；当前代码在结果搬回 CPU 前结束计时 |
| `peak_mem_mb` | CUDA 峰值 | Orin NX 的硬约束 |

### 4.2 怎么测

```bash
# 只测生成，不解码（分离瓶颈）
python bench/run_bench.py --cases bench/cases/text_cases.jsonl --device cpu --no-audio-decode --tag cpu-gen

# 生成 + 离线 Mimi 解码；仍不含 VAD、实时播放等交互链路
python bench/run_bench.py --cases bench/cases/text_cases.jsonl --device cuda --tag 3090-full

# 聚合
python bench/report.py bench/results/<run>/metrics.jsonl
```

### 4.3 结果怎么读

判断顺序（不要一上来就怀疑模型）：

1. 核对设备、精度、输入长度与冷/热启动口径，再比较生成耗时；CPU 结果同样可以作为该设备的有效基线。
2. 将 `ttfa - ttft` 与实际生成步数、思考模式及每步耗时对照，不把它直接称为用户首声延迟。
3. 语速或时长异常时，检查音频停止、漏读、重复、长度上限和实际 PCM；不预设 4 字/秒。
4. 分开定位生成与解码，并在交互测量中增加首块 PCM、播放首声、块间隔与欠载指标。平均吞吐足够也不保证每一块都及时到达。

### 4.4 用例为什么是这 30 条

不是通用百科，而是**机器人会真的听到的句式**：短指令、长指令、否定、数字、多轮、闲聊、状态查询、长回答。

理由：0.1B 模型的短板在知识，不在句式。用百科题测它，你只能得到"它很笨"这一个结论；用机器人句式测，你能得到"哪类句式会崩"的分布——后者才是工程决策需要的。

---

## 5. 本课实验

> 实验记录直接写在本节下面，格式：命令 → 结果表 → 结论一句话。

### 实验 1：延迟分解

```bash
python bench/run_bench.py --cases bench/cases/text_cases.jsonl --device cpu --tag exp1-cpu
```

待填：

| 分组 | ttft p50 | ttfa p50 | rtf p50 | 备注 |
|---|---:|---:|---:|---|
| short_cmd | | | | |
| long_cmd | | | | |
| ALL | | | | |

结论：

### 实验 2：文本与音频长度对照

同上一次结果，分别记录文本 token 数、音频帧数及二者的结束时刻，再结合 `chars_per_sec` 和试听解释节奏。

待填：

| 用例 | n_text_tokens | n_audio_frames | chars_per_sec | text_end_ms / audio_end_ms | 试听观察 |
|---|---:|---:|---:|---|---|
| 待填 | | | | | |

结论：

### 实验 3：`eval_omni.py` 与 `bench` 的口径差异

两处有意的差异，会造成数字不可比：

| 项 | `eval_omni.py` | `bench/common.py` |
|---|---|---|
| dtype | 无条件 `.half()` | CPU 用 float32，CUDA 用 float16 |
| 特殊 token | 只把 `>=2049` 归零 | `>=2048` 归零 |
| 解码 | 整段解码 + 转 mp3 | 整段解码，只存 wav |

实验：同一台机器、同一个权重、同一条 prompt，各跑一次，比较 `ttfa` 和音频时长。

结论：

### 实验 4（可选）：Orin NX 真机

```bash
python bench/run_bench.py --cases bench/cases/text_cases.jsonl --limit 5 --device cuda --tag orin-nx
```

待填：RTF、峰值显存、是否出现抖动（p95/p50 比值）。

---

## 6. 产出

本课结束时应留下：

- [ ] `bench/results/<run>/metrics.jsonl` 至少两份（CPU + 目标设备）
- [ ] 一张延迟分解表（按 category 分组）
- [ ] 一份 badcase 清单（哪条最差、差在哪：漏词/重复/语速/发音）
- [ ] 一句结论：**这个模型现在能不能撑住机器人交互？瓶颈在生成还是解码？**

结论句式建议：

> 在 `<设备>` 上，`<模型>` 的 ttfa p50 为 `<X>` ms，rtf p50 为 `<Y>`。瓶颈在 `<Thinker/Talker/Mimi>`。对机器人交互而言 `<可接受/不可接受>`，因为 `<理由>`。

---

## 7. 自测

1. 为什么 `ttfa` 一定晚于 `ttft`？晚多少步，由哪个配置决定？
2. `bridge_layer=3` 意味着 Talker 拿到的是第几层的输出？如果 Thinker 加到 12 层，这个默认值还合适吗？
3. 8 层 codebook 为什么用共享 base + adapter，而不是 8 个独立 head？代价是什么？
4. 为什么不能从 tokenizer 的字节表示和 Mimi 帧率推导中文语速？文本 EOS 后音频还能继续吗？
5. 如果 `rtf_wall = 1.4`，你会先查哪三个地方？

---

## 8. 下一课

**第 2 课：Tokenizer 与序列**——6400 词表怎么切中英混说？chat template 里的 `open_thinking` 到底改了什么？为什么 `</think>\n\n` 会被当成三个 token 特判？

---

## 参考

- `model/model_omni.py`（461 行，本课主教材）
- `dataset/omni_dataset.py::__getitem__`（第 210–330 行，序列拼装）
- `bench/README.md`（指标口径与注意事项）
- [MiniMind-O Technical Report](http://arxiv.org/abs/2605.03937)
