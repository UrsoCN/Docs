---
created: 2020-01-23
modified: 2026-07-27
tags:
  - type/article
  - topic/ai
  - topic/ml
---
# Scaling Laws for Neural Language Models

> **论文信息**
> - 作者：Jared Kaplan, Sam McCandlish, Tom Henighan, Tom B. Brown 等 (OpenAI)
> - 发表：2020年1月 (arXiv:2001.08361)
> - 引用量：3,500+
> - 领域：语言模型 / Scaling Laws

## 核心论点

语言模型的交叉熵损失与模型大小 N、数据集大小 D、训练计算量 C 之间遵循精确的幂律关系（power-law），跨越多达 7 个数量级。模型架构细节（如深度 vs 宽度）在合理范围内影响极小。最有反直觉的结论：**最优计算效率训练应使用极大模型但训练远未收敛即停止**。

## 关键发现

1. **幂律缩放公式**（基于 WebText2 数据集）：
   - 模型参数：L(N) = (Nc/N)^αN，αN ≈ 0.076，Nc ≈ 8.8×10¹³
   - 数据集大小：L(D) = (Dc/D)^αD，αD ≈ 0.095，Dc ≈ 5.4×10¹³ tokens
   - 最优计算量：L(Cmin) = (Cc/Cmin)^αC，αC ≈ 0.050，Cc ≈ 3.1×10⁸ PF-days

2. **模型大小比数据更重要**：性能对模型大小最敏感。每增加 8× 模型大小，只需约 5× 数据即可避免性能惩罚。过拟合水平与 N^0.74/D 成可预测关系。

3. **大模型样本效率更高**：大模型用更少的优化步骤和数据点即可达到相同性能水平。达到给定 loss 所需的 token 数与模型大小呈反比关系。

4. **最优批大小遵循幂律**：Bcrit(L) ≈ B*/L^(1/αB)，B* ≈ 2×10⁸ tokens，αB ≈ 0.21。收敛时最优批大小约为 1-2M tokens。

5. **迁移性能与训练性能高度相关**：在不同分布上的测试结果与训练验证集损失存在大致恒定的偏移，即迁移到不同分布只会带来固定 penalty。

6. **训练曲线可预测**：训练曲线的早期部分可外推预测长期训练的最终损失，且参数与模型大小几乎无关。

## 方法论

| 维度 | 详情 |
|------|------|
| 架构 | Transformer，参数范围从 10³ 到 10⁹（非嵌入） |
| 训练数据 | WebText2（GPT-2 使用的数据集） |
| 损失函数 | 自回归交叉熵 |
| 缩放范围 | 模型大小跨 6 个数量级，计算量跨 8 个数量级 |
| 控制变量 | 深度、宽度、注意力头数、前馈层维度 |

## 局限性

- 实验基于 Transformer + WebText2，数值常数依赖 vocabulary/tokenization，不具普遍意义
- 仅研究交叉熵损失，未涉及下游任务性能
- 未考虑推理效率或部署成本
- 后续研究（如 Chinchilla 论文，2022）修正了数据/模型大小的最优配比——Kaplan 认为模型增长应快于数据增长，Chinchilla 发现两者应等比增长

## 历史影响

这篇论文直接塑造了 GPT-3、PaLM 等大模型的研发思路，奠定了"规模即一切"（Scale is All You Need）的技术信仰。其发现——大模型样本效率更高——是后来 Scaling 竞赛的核心理论依据。2022 年 DeepMind 的 Chinchilla 论文（Training Compute-Optimal Large Language Models）修正了数据/模型的最优配比，但基本方法论继承自本文。

## 代表性引用

> "Performance depends strongly on scale, weakly on model shape."

> "Larger models are significantly more sample-efficient, such that optimally compute-efficient training involves training very large models on a relatively modest amount of data and stopping significantly before convergence."
