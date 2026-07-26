---
created: 2014-12-14
modified: 2026-07-27
tags:
  - type/article
  - topic/ai
  - topic/ml
---
# Sequence to Sequence Learning with Neural Networks

> **论文信息**
> - 作者：Ilya Sutskever, Oriol Vinyals, Quoc V. Le (Google)
> - 发表：NeurIPS 2014 (arXiv:1409.3215)
> - 引用量：25,000+
> - 领域：机器翻译 / 序列建模

## 核心论点

首次提出端到端的序列到序列（Seq2Seq）学习框架：用两个多层 LSTM 分别作为编码器和解码器，将变长输入序列映射为变长输出序列，无需对序列结构做任何假设。

## 关键发现

1. **Seq2Seq 架构有效击败传统方法**：在 WMT'14 英法翻译任务上，5 个 LSTM 的集成模型获得 BLEU 34.81，超越基于短语的 SMT 基线（33.30），这是纯神经网络翻译系统首次在大规模 MT 任务上超越 SMT。

2. **反转源句是关键的简单技巧**：将源句词序反转（`c, b, a → α, β, γ`）使 BLEU 从 25.9 跃升至 30.6。原因是缩短了源句和目标句对应词之间的"最小时间滞后"，引入了大量短程依赖，极大简化了优化问题。

3. **深层 LSTM 显著优于浅层**：4 层 LSTM 每增加一层，perplexity 降低近 10%。最终模型 384M 参数，8000 维隐藏状态。

4. **长句不退化**：LSTM 在长句上表现良好，与 Cho et al. 和 Bahdanau et al. 报告的长句性能下降相反。反转源句使 LSTM 获得了更好的记忆利用率。

5. **语义向量空间**：LSTM 学习到的句子表示对词序敏感，对主动/被动语态相对不变，PCA 可视化显示语义相似的句子聚类在一起。

## 方法论

| 维度 | 详情 |
|------|------|
| 架构 | Encoder-Decoder，4 层 LSTM，每层 1000 单元 |
| 参数量 | 384M（编码器 32M + 解码器 32M 循环连接） |
| 词表 | 源语言 160K，目标语言 80K |
| 训练数据 | WMT'14 英法，12M 句对（348M 法语词 + 304M 英语词） |
| 优化 | SGD 无动量，lr=0.7，每半 epoch 减半，共 7.5 epochs |
| 硬件 | 8 GPU 并行（4 GPU 给 LSTM 层 + 4 GPU 给 softmax） |
| 训练时间 | 约 10 天 |

## 局限性

- 固定大小的上下文向量成为信息瓶颈（后续 Bahdanau et al. 通过注意力机制解决）
- 词汇表外（OOV）词用 UNK 替代，无法生成
- 仅在大规模平行语料可用时有效
- 注意力机制问世后（同年 Bahdanau et al.），纯 fixed-vector 方案很快被超越

## 历史意义

这篇论文是深度学习中最重要的里程碑之一，奠定了现代 NLP 的基础范式。其 Encoder-Decoder 架构成为机器翻译、文本摘要、对话系统、语音识别等几乎所有序列转换任务的标准范式。尽管 Google Brain 的 "Attention Is All You Need"（2017）后来用 Transformer 取代了 LSTM，Seq2Seq 的编码-解码思想至今仍是主流架构的核心。

## 代表性引用

> "The LSTM did not have difficulty on long sentences... reversing the order of the words in the source sentence is one of the key technical contributions of this work."

> "A straightforward application of the Long Short-Term Memory architecture can solve general sequence to sequence problems."
