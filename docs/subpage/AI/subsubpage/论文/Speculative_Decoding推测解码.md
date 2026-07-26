---
created: 2022-11-30
modified: 2026-07-27
tags:
  - type/article
  - topic/ai
  - topic/ml
---
# Fast Inference from Transformers via Speculative Decoding

> **论文信息**
> - 作者：Yaniv Leviathan, Matan Kalman, Yossi Matias (Google Research)
> - 发表：ICML 2023 (arXiv:2211.17192)
> - 引用量：1,000+
> - 领域：LLM 推理加速

## 核心论点

利用**推测执行**（speculative execution）思想加速自回归模型推理：用一个高效的小模型生成 γ 个候选 token，再用大模型并行验证这些 token。通过一种新颖的**推测采样**（speculative sampling）方法，保证输出分布与原始大模型完全相同，无需修改模型架构或重新训练。

## 关键发现

1. **无需改模型、无需重训练、输出分布不变**：这是推测解码最大的工程价值——可以直接加速现成的 off-the-shelf 模型。

2. **2×-3× 实测加速**：在 T5-XXL（11B）上对比 T5X 官方实现，实现了端到端 2×-3× 的 walltime 加速。

3. **理论保证输出一致**：通过推测采样（speculative sampling）算法，数学上保证加速后的输出分布与原始目标模型完全一致，不是近似或蒸馏。

4. **加速原理**：大模型推理的瓶颈通常不是算术运算，而是**内存带宽**和通信。推测解码增加了并发性（并行验证多个 token），利用了闲置的计算资源。

5. **效果与任务难度相关**：简单任务（如无条件生成、常见搭配）加速效果更好，复杂任务（如需要专业知识的翻译）加速效果略低，但仍有显著提升。

## 推测采样算法（核心）

1. 用小模型 Mq 自回归生成 γ 个候选 token x₁,...,x_γ
2. 用大模型 Mp **并行**计算这些 token 在自身分布下的概率 p(x_i) 和小模型概率 q(x_i)
3. 对每个位置 i，以概率 min(1, p(x_i)/q(x_i)) 接受该 token
4. 若某个 token 被拒绝，用调整后的分布 (p − q)_+ 重新采样该位置，并丢弃后续所有候选
5. 接受的所有 token 作为输出，从新采样的 token 继续下一轮

## 方法论

| 维度 | 详情 |
|------|------|
| 目标模型 | T5-XXL (11B)、LaMDA (137B)、GPT-like (97M) |
| 近似模型 | 参数量小 2-3 个数量级的同架构模型 |
| 验证任务 | 无条件生成、英德翻译、新闻摘要、对话 |
| 最优 γ | 取决于大小模型的速度比和任务难度，通常 3-5 |

## 局限性

- 需要维护一个额外的近似模型，增加内存占用
- 近似模型必须与目标模型使用相同的 tokenizer
- 在计算瓶颈场景（而非内存带宽瓶颈）下效果有限
- 加速比有理论上限，取决于大小模型的接受率

## 后续发展

推测解码已成为 LLM 推理加速的标准技术之一。后续工作包括：
- **Medusa**（Zhao et al., 2024）：用多个独立注意力头直接预测多个 future token
- **EAGLE / EAGLE-2**：用特征级别的推测解码进一步提升接受率
- **Self-Speculative Decoding**：MTP 论文（Gloeckle et al., 2024）用训练好的多 token 预测头实现自推测解码

## 代表性引用

> "Our method can accelerate existing off-the-shelf models without retraining or architecture changes."

> "We generalize speculative execution to the stochastic setting... guaranteeing that the outputs from our system have the same distribution as those from the target model alone."
