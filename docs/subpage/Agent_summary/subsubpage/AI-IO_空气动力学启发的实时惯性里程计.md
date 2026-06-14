---
created: '2026-06-09T00:00:00.000+0800'
modified: '2026-06-09T00:00:00.000+0800'
tags:
  - ai
  - robot
  - SLAM
  - ICRA
  - 里程计
  - IMU
  - Transformer
  - EKF
---
# AI-IO: 空气动力学启发的实时惯性里程计

> **Acronym：** AI-IO (Aerodynamics-Inspired Inertial Odometry)
> **作者：** 崔嘉浩、余锋、张麟佐、胡宇、邹丹平 — 上海交通大学 ViSYS 实验室
> **发表：** ICRA 2026
> **代码：** [github.com/SJTU-ViSYS-team/AI-IO](https://github.com/SJTU-ViSYS-team/AI-IO)
> **论文：** arXiv: 2603.00597

---

## 1. 核心创新

传统惯性里程计（纯 IMU 加速度+陀螺仪积分）漂移严重，核心问题在于**加速度计无法区分自身加速度与重力**，导致积分前存在不可观测的偏差。

AI-IO 的关键洞察：**四旋翼的旋翼转速（rotor speed）通过空气动力学模型与飞行速度直接相关**，引入这一额外的物理信号后，速度预测的精度提升 **36.9%**。

> 对于四旋翼，旋翼转速是可观测的 ESC 反馈信号，其平方值与推力成正比，在多旋翼动力学中与体速度存在可学习的映射关系。

---

## 2. 整体架构（开源实现分析）

### 2.1 系统流程图

```
IMU (acc + gyro) ────┐   ┌─ 1D Conv (kernel=5, 3→16)
                      ├──→├─ 1D Conv (kernel=5, 3→16) ─┐
旋翼转速 (4轴) ───────┘   └─ 1D Conv (kernel=5, 4→16) ─┴→ Concat [B,T,48]
                                                              │
                                                         PosEncoding
                                                              │
                                                    TransformerEncoder
                                                     (2层, nhead=8, FF=256)
                                                              │
                                                     ┌────────┴────────┐
                                                     ▼                 ▼
                                              Linear → v_body(3)   Linear → log σ(3)
                                                     │                 │
                                                     └─────┬──────────┘
                                                           ▼
                                                    EKF Update @20Hz
                                                           │
                                                    State: R, v, p, ba, bg
```

### 2.2 模型设计实现

**代码位置：** `src/learning/network/model.py`

核心类 `IMUTransformerWithModality`：

- **输入：** 10 通道时间序列 [acc(3), gyro(3), rotor_spd(4)]，时间窗口 **1 秒**（默认 100Hz → 100 个时间步）
- **模态编码层：** 每个模态分别过独立的 1D Conv（kernel=5, stride=1），各映射到 **16 维**子空间 → 拼接为 48 维特征
- **旋翼转速特殊处理：** 取平方后再接 `EmpiricalNormalization`（在线统计均值和方差进行标准化）
- **位置编码：** `PositionalEncoding`（最大支持 500 步，sin/cos 固定编码）
- **Transformer 编码器：**
  - 2 层，nhead=8，dim_feedforward=256，dropout=0.2
  - 自定义 `TransformerEncoderLayerWithAttention`：推理时保存 attention 权重用于可视化
  - 输入格式 `[T, B, C]`（PyTorch 原生 `nn.TransformerEncoder` 要求）
  - 取**最后一个时间步**的隐状态做预测
- **输出头：** 两个 Linear 分支：
  - `output_head` → 体速度预测 `v_body` [B, 3]
  - `output_cov` → 对数标准差 `log σ` [B, 3]（进入 `exp(2*logσ)` 转对角协方差）

**模型参数总数：** ~2.3M 参数（轻量级）

### 2.3 对比实验的 TCN 基线

代码中也提供了 TCN（Temporal ConvNet）实现作为基线（`Tcn` 类）：
- 4 层空洞卷积（dilation=1, 2, 4, 8）
- 通道数 [64, 64, 128]，kernel=2
- GELU 激活 + WeightNorm + Residual 连接
- 感受野 ≈ 1 + 2×(2-1)×(2⁴-1) = 31 步 ≈ 0.31s

Transformer 比 TCN **误差降低 22.4%**。

### 2.4 训练实现

**代码位置：** `src/learning/train_model_net.py` + `src/main_learning.py`

**超参数：**
| 参数 | 值 |
|---|---|
| 优化器 | Adam，lr=3e-4 |
| 调度器 | ReduceLROnPlateau (factor=0.1, patience=5) |
| 批次 | 128 |
| 轮数 | 60 |
| 损失: Phase 1 (epoch<50) | Huber loss (δ=0.1 m/s) |
| 损失: Phase 2 (epoch≥50) | 负对数似然 (NLL) + 协方差学习 |

**两阶段训练策略：**
1. **Phase 1（前 50 轮）：** 只用 Huber 损失，让网络先稳定地学会速度预测
2. **Phase 2（切换后）：** 切换到 NLL + 协方差头一起优化，让网络学会不确定性建模

> `switch_iter=50` 控制切换时机，通过 `get_loss()` 中的 epoch 判断实现

**协方差参数化：**
- 默认 `DiagonalParam`：3 个参数 `log σ_x, log σ_y, log σ_z`
- 可选 `PearsonParam`：6 个参数，增加相关系数 `ρ_xy, ρ_xz, ρ_yz`（使用 `tanh` 限制在 ±(1-ε) 内）
- NLL 损失：`loss = (pred-targ)² / (2·exp(2·logσ)) + logσ`
- 防止除零：`MIN_LOG_STD = ln(0.001)`

**数据增强：** 可选的加速度扰动 `--perturb_accel`（默认关闭，±0.1 m/s² 均匀噪声）

### 2.5 数据格式

**代码位置：** `src/learning/data_management/datasets.py`

输入是预处理的 **HDF5 文件**（`data.hdf5`）：
| 字段 | 维度 | 说明 |
|---|---|---|
| `ts` | [N] | 时间戳 |
| `accel_calib` | [N, 3] | 标定后加速度 (m/s²) |
| `gyro_calib` | [N, 3] | 标定后角速度 (rad/s) |
| `rotor_spd` | [N, 4] | 旋翼转速 (rad/s) |
| `traj_target` | [N, 10] | 真实轨迹: t(1) + q(4) + p(3) + v(3) |

**训练标签：** 将全局速度 `v_world` 通过姿态旋转矩阵投影到机体坐标系：
```python
v_body = R_world^T @ v_world
```

**滑动窗口：**
- 窗口长度：`window_size = sampling_freq × window_time`（100Hz × 1s = 100 步）
- 窗口步长：`window_shift_size = 1`（每步滑动 1 个采样点）

### 2.6 EKF 滤波器实现

**代码位置：** `src/filter/python/src/scekf.py`

**状态量（15 维）：**

| 分量 | 维度 | 含义 |
|---|---|---|
| θ (R) | 3 | 姿态误差角（so(3) 参数化） |
| v | 3 | 全局速度 (m/s) |
| p | 3 | 全局位置 (m) |
| bg | 3 | 陀螺仪零偏 (rad/s) |
| ba | 3 | 加速度零偏 (m/s²) |

**传播（IMU 输入，100Hz）：**
```
R_{k+1} = R_k · exp((ω_m - bg)·dt)
v_{k+1} = v_k + R_k·(a_m - ba)·dt + g·dt
p_{k+1} = p_k + v_k·dt + 0.5·R_k·(a_m - ba)·dt² + 0.5·g·dt²
```

连同解析雅可比矩阵 `A`（15×15）+ 噪声协方差传播，使用 **numba JIT** 编译加速。

**更新（网络预测，20Hz）：**
1. 从 `NetInputBuffer` 获取最近 1 秒插值后的 IMU + 旋翼转速数据
2. 运行 Transformer 推理 → 得到 `v_body_pred` + `Cov_meas` (3×3)
3. EKF 更新步骤：
   - 预测：`v_body_predicted = R^T · v`
   - 创新：`z = v_body_pred - v_body_predicted`
   - 雅可比 H 矩阵：对姿态 (3×3 rotation) 和速度 (3×3 identity)
   - 马氏距离门控（可选）过滤异常值
4. 更新后的 15 维误差状态通过 `apply_correction` 修正名义状态

**可选配置：** 常值协方差模式（`use_const_cov`）跳过网络不确定性输出，使用预设常数

**不可观测方向分析：**
- 偏航角 + 全局位置 3DOF 不可观
- 通过 `generate_unobservable_shift()` 计算零空间

### 2.7 运行流程

```
main_filter.py
    ↓ 读取 data_config (.conf) + checkpoint(.pt)
FilterManager
    ↓ 逐序列遍历
DataIO → 读取 HDF5 / 文本数据
    ↓ 单步：ts, acc_raw, gyr_raw, rotor_spd
FilterRunner.on_imu_measurement()
    ├─ 初始化（首次调用）：重力对齐 R + 零初速度 + 离线标定偏置
    ├─ 插值：测量值线性插值到目标频率 → NetInputBuffer
    ├─ 传播：ImuMSCKF.propagate() → state + covariance
    ├─ 更新（每 5 个 IMU 步 = 20Hz）：
    │   ├─ 取 buffer 中 1s 窗口数据
    │   ├─ MeasSourceNetwork.get_measurement() → v_body, Cov
    │   └─ ImuMSCKF.learnt_model_update() → EKF correction
    └─ 日志：ts, p, q, v, ba, bg → 文本文件
```

### 2.8 推理延迟

部署在 **Radxa Zero 3W**（4×Arm Cortex-A55 @1.6GHz）：
- Transformer 推理：**8.9ms**（远小于 50ms 的 20Hz 更新周期）
- EKF 更新：<1ms
- 完全满足实时运行要求

### 2.9 精度（DIDO 数据集）

| 指标 | SOTA (AirIO) | AI-IO | 改善 |
|---|---|---|---|
| 速度误差 (AVE) | 1.182 m/s | **0.371 m/s** | **68.6%** |
| 位置误差 (ATE) | 5.972 m | **3.876 m** | **35.1%** |
| 5s 滑动平移 (RTE) | 2.314 m | **0.903 m** | **61.0%** |

> 整体声明：速度精度提升 49.4%，位置精度提升 30.9%

### 2.10 代码结构总览

```
AI-IO/
├── config/                    # 数据配置文件（按训练/验证/测试分 section）
│   └── our2.conf (DIDO.conf)
├── src/
│   ├── main_learning.py       # 训练入口（train/test 模式）
│   ├── main_filter.py         # EKF 滤波入口（离线跑序列）
│   ├── learning/
│   │   ├── data_management/
│   │   │   ├── datasets.py    # Dataset 类，HDF5 加载 + 滑动窗口采样
│   │   │   └── prepare_datasets/  # 原始数据预处理
│   │   ├── network/
│   │   │   ├── model.py       # Transformer(主) + TCN(基线)
│   │   │   ├── model_factory.py   # 模型工厂
│   │   │   ├── losses.py      # Huber / MSE / NLL 损失
│   │   │   └── covariance_parametrization.py  # 协方差参数化
│   │   ├── train_model_net.py # 训练循环
│   │   ├── test_model_net.py  # 测试循环
│   │   └── utils/             # 日志、绘图、位姿、参数转换
│   └── filter/python/
│       ├── src/
│       │   ├── scekf.py       # MSCKF EKF 核心（15-state, JIT）
│       │   ├── filter_runner.py    # 协调 IMU 输入 + 网络推理 + EKF 更新
│       │   ├── filter_manager.py   # 序列级循环管理器
│       │   ├── meas_source_network.py  # 加载 checkpoint 做推理
│       │   ├── net_input_utils.py     # IMU 输入缓冲区 + 插值 + 标定
│       │   ├── data_io.py     # 数据 I/O
│       │   └── utils/         # 数学工具、参数处理、绘图
│       └── python/plot_filter_output.py  # 结果可视化
└── requirements.txt
```

---

## 3. 迁移到机器狗的关键差异

### 3.1 四旋翼 vs 机器狗的物理量映射

| 四旋翼 (论文) | 机器狗 (你) |
|---|---|
| 旋翼转速 (ESC 反馈, 4 轴) | 关节编码器 + 电机电流 (每腿 3+ 关节) |
| 桨叶空气动力学 → 推力 → 速度 | 足端接触 + 关节运动 → 本体速度 |
| 连续的平滑运动 | 周期性的触地冲击 + 摆动相 |
| 推力 ≈ 转速² × 常数 | 地面反作用力 ≈ 关节扭矩/力（高度非线性） |

### 3.2 架构改造建议

如果要在机器狗上借鉴此方法：

```
IMU (加速度+角速度) ────┐
                         ├─→ Transformer ─→ v_body_pred ─→ EKF ─→ 6-DoF 位姿
关节状态 (位置/速度/扭矩) ─┘       ↑
足端接触传感器 ────────────────────┘
```

需要调整的部分：

| 论文组件 | 机器狗上的对应改动 |
|---|---|
| 模态数: 3 (acc/gyro/rotor) | 4+ 模态: acc(3) + gyro(3) + joint_pos(N) + joint_vel(N) + contact(N) |
| 旋翼平方+标准化 | 关节状态需**周期感知**（将步态周期编码进输入），或使用顺滑的足端里程计辅助信号 |
| 连续窗口 1s @100Hz | 建议增加窗口至 1.5-2s 覆盖完整步态周期 |
| 20Hz 更新 | 足式可降至 10-20Hz（步态周期更长） |
| 训练数据: 飞行 6.7km 动捕 | 需要采集多种步态（走/跑/转向/上下坡）的动捕标注数据 |

### 3.3 已知相关工作

- **Buchanan et al., CoRL 2022** — IMU + 关节编码器 + 学习模型 + 批量优化，比纯运动学-惯性融合降低 **37%** 位姿误差
- **Deep IMU Bias Inference (DIBI)** — 学习 IMU 偏置估计用于足式里程计
- **状态估计器 + 足端里程计是工业机器狗标配**：接触已知时用正向运动学 + IMU 积分，接触未知时（打滑、弹跳）用学习模型替代

---

## 4. 开源可用性

- **GitHub 仓库：** https://github.com/SJTU-ViSYS-team/AI-IO
- **预训练权重：** `checkpoint_open.pt`（已发布在 Release v1.0）
- **数据集：** `AI-IO_dataset.tar.gz`（官方室内动捕数据集，含 manual / eight / circle / random 四种轨迹 × 高/中/低三种速度）
- **DIDO 数据集：** https://github.com/zhangkunyi/DIDO
- **环境：** Python 3.10 + PyTorch + requirements.txt
- **硬件验证：** Radxa Zero 3W（Arm 开发板）实测 8.9ms 推理延迟
