---
created: 2024-04-30
modified: 2026-07-27
tags:
  - type/article
  - topic/ai
  - topic/ml
---
# Better & Faster Large Language Models via Multi-token Prediction

> **论文信息**
> - 作者：Fabian Gloeckle, Badr Youbi Idrissi, Baptiste Rozière, David Lopez-Paz, Gabriel Synnaeve (Meta FAIR)
> - 发表：2024年4月 (arXiv:2404.19737)
> - 领域：LLM 训练方法 / 推理加速

## 核心论点

将 LLM 的训练目标从单 token 预测（next-token prediction）扩展为**多 token 预测**（multi-token prediction）：在训练语料的每个位置，要求模型同时预测未来 n 个 token。该方法作为辅助训练任务，无需额外训练时间或显存开销，能显著提升下游任务性能，且在推理时可用于自推测解码（self-speculative decoding）实现最高 3× 加速。

## 关键发现

1. **代码任务大幅提升**：13B 模型在 HumanEval 上比 next-token 基线多解决 **12%** 更多问题，MBPP 上多 **17%**。generative benchmark 上的提升最为显著。

2. **模型越大，收益越明显**：多 token 预测的增益随模型规模增长而增大，13B 级别效果最强。

3. **零训练时间开销**：通过精心设计前向/反向传播顺序，避免了同时物化 n 个 logit 张量（大小 n×V），GPU 显存利用率不降低。

4. **推理加速：自推测解码**：推理时使用额外的输出头作为"免费"的推测解码草稿模型，无需维护独立的近似模型。4-token 预测训练的模型推理速度可提升 **3×**。

5. **促进 Induction Head 形成**：在小规模算法任务上的实验表明，多 token 预测有利于诱导头（induction heads）和算法推理能力的发展。

6. **多轮训练仍然有效**：在多个 epoch 的训练设置下，多 token 预测仍保持优势。

## 架构设计

| 组件 | 详情 |
|------|------|
| 共享主干 | Transformer trunk fs，生成隐藏表示 z |
| n 个独立输出头 | n 个 Transformer 层 fhi，每个预测一个 future token |
| 共享反嵌入矩阵 | 所有头共用 fu（vocabulary projection） |
| 训练损失 | Ln = -Σ_t Σ_i log Pθ(x_{t+i} | z_{t:1}) |
| 推理 | 仅使用 next-token head（i=1），可选启用其他 head 做自推测解码 |

## 与 Speculative Decoding 的关系

传统的 Speculative Decoding（Leviathan et al., 2022）需要维护一个独立的近似模型。MTP 通过在训练时多出 n-1 个输出头，推理时这些头可直接作为"草稿模型"使用——这就是**自推测解码**（self-speculative decoding），无需额外模型，是更优雅的方案。

## 局限性

- 最优预测 token 数量 n 需要针对任务和模型规模调参
- 额外输出头增加了一定参数量（n 个 Transformer 层）
- 论文未在超过 13B 的规模上验证
- 主要验证在代码任务上，自然语言生成任务的优势相对较小
- 与 next-token 预测相比的理论性质（如收敛性）尚未充分研究

## 实际意义

MTP 是一项训练成本极低的改进——不增加训练时间、不增加显存、只需多几个输出头。对于需要 LLM 推理服务部署的团队，额外输出头可以在推理时带来显著的吞吐提升。该方法已被多篇后续工作采用和改进。

## 代表性引用

> "Training language models to predict multiple future tokens at once results in higher sample efficiency."

> "Multi-token prediction enables self-speculative decoding, making models up to 3× faster at inference time."
