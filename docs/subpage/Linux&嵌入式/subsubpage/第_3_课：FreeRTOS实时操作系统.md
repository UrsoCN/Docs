---
created: 2026-07-13
tags: [embedded, esp32, learning, xvf3800, lesson-3]
---

# 第 3 课：FreeRTOS 实时操作系统

> 核心问题：当系统需要同时做多件事，且每件事的响应时间可预测时——怎么做？
> 答案：RTOS。把代码拆成独立任务，由调度器决定谁在哪个核心上跑，什么时机跑。

---

## 一、为什么需要 OS？

裸板开发的本质是一个 `while(1)` 主循环 + 中断服务程序。在小项目里够用，但随着需求变复杂，问题随之而来：

```mermaid
flowchart TD
    A["问题 1: 阻塞死等<br/>while(!spi_ready()) ; // CPU 空转"]
    B["问题 2: 响应延迟不可控<br/>串口来数据了<br/>但主循环还在处理 I2S 缓冲"]
    C["问题 3: 代码耦合<br/>串口处理里嵌着 OTA 逻辑<br/>改一处影响全局"]
    D["问题 4: 团队协作困难<br/>多个开发者同时改一个主循环"]
```

**RTOS 解决的终极问题**：让你"同时做多件事"，且每件事的响应时间**可确定地预测**。

| | 裸板 (Bare-metal) | RTOS |
|------|:---:|:---:|
| **并发方式** | 手动在循环里轮询 | 多任务，由调度器自动切换 |
| **响应延迟** | 取决于循环周期 | **确定性的** — 高优先级就绪立刻抢占 |
| **阻塞操作** | CPU 空转死等 | **任务阻塞时 CPU 自动切走** |
| **代码组织** | 一个大 `while(1)` | 每个任务是一个独立函数 |
| **RAM 占用** | 极小（几 KB） | 中等（几十 KB） |
| **动态调试** | 串口 printf 打天下 | 有任务列表、栈使用统计等辅助手段 |

> **关键理解**：RTOS 不加速代码的执行速度——它**让 CPU 在多个任务间高效分配时间**，使系统整体响应性更好。

---

## 二、任务（Task）—— RTOS 的基本单元

任务是 RTOS 中独立执行的"线程"。每个任务是一个**无限循环函数**，有单独的栈空间。

```c
// 一个任务的基本形态
void my_task(void *pvParameters)
{
    while (1) {
        // ① 等待事件（阻塞，不消耗 CPU）
        wait_for_something();
        // ② 处理事件
        do_work();
    }
}
```

### 2.1 创建任务

本项目所有任务使用 `xTaskCreatePinnedToCore`（ESP-IDF 扩展版，多了核心号参数）：

```c
BaseType_t xTaskCreatePinnedToCore(
    TaskFunction_t pvTaskCode,      // 任务函数指针
    const char *const pcName,       // 任务名（调试用，最多 16 字符）
    configSTACK_DEPTH_TYPE usStackDepth, // 栈大小（单位：字，不是字节！）
    void *pvParameters,             // 传入参数
    UBaseType_t uxPriority,         // 优先级（0~24，数字越大优先级越高）
    TaskHandle_t *pxCreatedTask,    // 返回的任务句柄（可选 NULL）
    BaseType_t xCoreID              // 运行在哪个核心（0/1/tskNO_AFFINITY）
);
```

> **栈大小 (usStackDepth) 的单位是"字"不是"字节"**。ESP32-S3 是 32 位架构，1 字 = 4 字节。设 `4096` 就是 16KB 栈。

**常规版 `xTaskCreate`** 不带核心号参数，让调度器决定跑在哪个核上。本项目全部使用 `PinnedToCore` 版本——因为音频任务有严格的核心亲和性要求。

### 2.2 任务状态机

一个任务的生命有四种状态：

```mermaid
stateDiagram-v2
    [*] --> Ready: xTaskCreate
    Ready --> Running: 调度器选中
    Running --> Ready: 被抢占 / 时间片到
    Running --> Blocked: vTaskDelay / xQueueReceive / xSemaphoreTake
    Blocked --> Ready: 事件到达 / 超时
    Running --> Suspended: vTaskSuspend
    Suspended --> Ready: vTaskResume
```

| 状态 | 含义 | CPU 消耗 |
|------|------|:--------:|
| **Ready** | 准备好运行，在排队等 CPU | ❌（排队中） |
| **Running** | 正在占用 CPU（一个核上一刻只能有一个） | ✅ |
| **Blocked** | 在等某个事件（延时/信号量/队列），不消耗 CPU | ❌ |
| **Suspended** | 被显式挂起，永不调度除非主动恢复 | ❌ |

> `vTaskDelay(100 / portTICK_PERIOD_MS)` 的本质：任务进入 Blocked 状态 100ms——**CPU 不空转，去跑其他任务**。这就是 RTOS 比裸板 Super Loop 高效的根本原因。

---

## 三、调度器（Scheduler）—— 谁决定"下一步跑谁"

### 3.1 抢占式优先级调度

FreeRTOS 的默认调度策略（本项目所用的模式）：**高优先级任务就绪时，立刻抢占低优先级任务**。

```mermaid
sequenceDiagram
    participant T1 as 任务 A (prio 5)
    participant T2 as 任务 B (prio 3)
    participant SCH as 调度器

    T2->>SCH: Running
    Note over T2: 低优先级任务正在执行

    T1-->>SCH: 中断触发，任务 A 就绪
    SCH->>T2: 强制切出（抢占！）
    SCH->>T1: 切换到任务 A

    Note over T1: 高优先级任务执行或阻塞

    T1->>SCH: vTaskDelay(x) → Blocked
    SCH->>T2: 恢复任务 B
```

> **抢占的本质**：不是"协商"——低优先级任务**无权拒绝**。这就是 RTOS 实时性的根基。

### 3.2 同优先级：时间片轮转

相等优先级的任务共享 CPU，每个任务轮流跑一个时间片。

```
时间线: |---A---|---B---|---A---|---B---|---A---|---B---|
         tick0   tick1   tick2   tick3   tick4   tick5   tick6
```

### 3.3 系统节拍（Tick）—— 调度的心跳

FreeRTOS 使用一个硬件定时器（通常为 SysTick）周期性产生中断，每次中断就是一次"重新决策"的机会。

```c
// FreeRTOS 的 Tick ISR（简化示意）
void xPortSysTickHandler(void)
{
    portDISABLE_INTERRUPTS();       // ① 关中断保护
    xTaskIncrementTick();           // ② Tick 计数器 +1
    // ③ 检查是否有任务需要唤醒（vTaskDelay 到期？）
    // ④ 检查是否需要任务切换（更高优就绪？）
    // ⑤ 如需切换，触发 PendSV 异常（在异常退出时自动切换）
    portENABLE_INTERRUPTS();
}
```

**本项目配置**（来自 `sdkconfig.defaults`）：

| 配置项 | 值 | 含义 |
|--------|:---:|------|
| `CONFIG_FREERTOS_HZ` | 1000 | 系统心跳 1ms（Tick ISR 每秒跑 1000 次） |
| `CONFIG_FREERTOS_NUMBER_OF_CORES` | 2 | 双核对称多处理 |
| `CONFIG_FREERTOS_MAX_PRIORITIES` | 25 | 优先级范围 0~24（数字越大越高） |
| `CONFIG_FREERTOS_IDLE_TASK_STACKSIZE` | 1536 | 每个核心空闲任务的栈 |

> 1000 Hz 意味着调度器每秒做 1000 次"要不要换个任务跑"的决策。桌面 Linux 通常只设 250 Hz——嵌入式需要更快的反应。
>
> 这也是为什么 `vTaskDelay(1)` 最少延迟 1ms——它不是在硬件定时器上等 1ms，而是等下一次 Tick ISR 发现"该你了"。

---

## 四、本项目 7 个任务全景

这是 **XVF3800 ESP32-S3 固件** 全部的 FreeRTOS 任务，是任务设计的最佳实战案例：

```mermaid
flowchart TB
    subgraph CORE0[Core 0 — 音频处理核心]
        MIC[usb_mic_task<br/>prio 14<br/>I2S RX → USB IN]
        SPK[usb_spk_task<br/>prio 14<br/>USB OUT → I2S TX]
        DSP[ds_dsp<br/>prio 5<br/>NS/AGC + MultiNet KWS]
        WDOG[spk_wdog<br/>prio 3<br/>I2S TX 静音填充]
        OTA[ota_flash<br/>prio 3<br/>固件写入 SPI Flash]
    end

    subgraph CORE1[Core 1 — USB 协议栈]
        TUSB[TinyUSB<br/>prio 15<br/>USB 设备协议栈]
        VAD[doa_vad<br/>prio 5<br/>声源定位/人声检测]
    end

    SPK -.->|通知| TUSB
    MIC -.-> DSP
    DSP -.->|JSON 命令| CDC
```

| 任务名 | 函数 | 栈 | 优先级 | 核心 | 做什么 |
|--------|------|:--:|:-----:|:----:|--------|
| `doa_vad` | `doa_vad_task` | 4096 | 5 | **1** | 轮询 XVF3800 读取声源方向和人声状态，1 Hz |
| `TinyUSB` | `tusb_device_task` | 4096 | **15** | **1** | 运行 TinyUSB 设备协议栈（处理枚举/控制传输） |
| `usb_mic_task` | `usb_mic_task` | 4096 | **14** | **0** | I2S 麦克风数据 → USB IN 端点 |
| `usb_spk_task` | `usb_spk_task` | 4096 | **14** | **0** | USB OUT 端点 → I2S 扬声器 |
| `ds_dsp` | `dsp_consumer_task` | 4096 | **5** | **0** | NS 降噪 + AGC + MultiNet 关键词识别 |
| `spk_wdog` | `spk_watchdog_task` | 2048 | **3** | **0** | 无音频时填充静音，防止 I2S TX DMA 饥饿 |
| `ota_flash` | `ota_flash_task` | **8192** | **3** | **0** | 通过 CDC 接收固件数据，写入 SPI Flash |

### 设计分析

**Core 0（音频核心）的优先级金字塔**：
```
prio 14  usb_mic_task  +  usb_spk_task   ← 相等！音频 I/O 最高
prio  5  ds_dsp                           ← 音频处理，中等
prio  3  spk_wdog  +  ota_flash          ← 后台任务，最低
```

> **为什么 usb_mic_task 和 usb_spk_task 优先级相等？**
> 这来自项目开发中踩过的坑（见 AGENTS.md）：如果其中一个比另一个高，高的一方会大量抢占 CPU，导致另一方的 DMA 长期得不到服务——全双工模式下 TX 或 RX 就会失真。
>
> **相等 = 时间片轮转 = 谁都不饿死谁**。

**Core 1 的独立性**：
- `TinyUSB` 是最高优先级（15）——USB 协议栈需要及时响应 Host 的令牌和请求
- `doa_vad` 只在 Core 1 运行，与 Core 0 的音频处理互不干扰

### 任务创建背后的配置驱动

本项目中，小米（MIC）、扬声器（SPK）、TinyUSB 的优先级和核心亲和性**不写在代码里死**，而是通过 Kconfig 配置驱动：

```c
// 实际调用（来自 usb_device_uac.c）
ret_val = xTaskCreatePinnedToCore(usb_mic_task, "usb_mic_task", 4096, NULL,
    CONFIG_UAC_MIC_TASK_PRIORITY, &s_uac_device->mic_task_handle,
    CONFIG_UAC_MIC_TASK_CORE == -1 ? tskNO_AFFINITY : CONFIG_UAC_MIC_TASK_CORE);
```

对应的 `sdkconfig.defaults`（或通过 `idf.py menuconfig` 设置）：
```
CONFIG_UAC_TINYUSB_TASK_PRIORITY=15
CONFIG_UAC_TINYUSB_TASK_CORE=1
CONFIG_UAC_MIC_TASK_PRIORITY=14
CONFIG_UAC_MIC_TASK_CORE=0
CONFIG_UAC_SPK_TASK_PRIORITY=14
CONFIG_UAC_SPK_TASK_CORE=0
```

> 这是良好的工程实践：**运行时策略（优先级、核心分配）与业务逻辑分离**。调优优先级时不用改代码，只需改配置。

---

## 五、队列（Queue）—— 任务间发送数据

任务是隔离的——各跑各的循环，有各自的栈。它们怎么互相传递数据？**队列**。

### 5.1 基本用法

```c
// 创建队列：能存 10 个 uint32_t
QueueHandle_t queue = xQueueCreate(10, sizeof(uint32_t));

// 发送方（任务 A）
uint32_t data = 42;
xQueueSend(queue, &data, portMAX_DELAY);   // 一直等到有空位

// 接收方（任务 B）
uint32_t received;
xQueueReceive(queue, &received, portMAX_DELAY);  // 一直等到有数据
// received == 42
```

```mermaid
flowchart LR
    subgraph TASKA[任务 A — 发送方]
        SEND[xQueueSend]
    end
    subgraph QUEUE[消息队列 FIFO]
        D4[42] --> D3[7] --> D2[3] --> D1[99]
    end
    subgraph TASKB[任务 B — 接收方]
        RECV[xQueueReceive]
    end
    SEND -->|入队| D4
    D1 -->|出队| RECV
```

### 5.2 阻塞与超时

`xQueueReceive` 的第三个参数 `xTicksToWait` 是最重要的设计：

| 超时值 | 行为 |
|--------|------|
| `0` | 非阻塞：有数据就取，没数据立刻返回 |
| `pdMS_TO_TICKS(10)` | 等 10 个 tick（10ms），超时仍没数据就返回 `errQUEUE_EMPTY` |
| `portMAX_DELAY` | 永久阻塞，直到有数据 |

> 这不是 `while(queue_empty()) ;` 那种忙等——**任务在 Blocked 状态，CPU 被切走做其他事**。阻塞期间 0% CPU 浪费。

### 5.3 本项目队列实例：音频帧传递

本项目通过队列将 USB 麦克风的音频帧传递给 DSP 处理任务：

```c
// 创建队列（xvf_downsample.c:146）
s_frame_queue = xQueueCreate(4, sizeof(ds_work_item_t));
// 最多缓存 4 帧（每帧 160 样本/320 字节，16 bit @ 16kHz）
// 队列深度 = 4 意味着最多缓冲 40ms 音频

// USB MIC 回调（ISR 上下文）：非阻塞发送
// 收到新音频帧 → 入队
if (xQueueSend(s_frame_queue, &work_item, 0) != pdTRUE) {
    // 队列满了 → 丢掉最老帧（音频流不能阻塞中断）
}

// ds_dsp 任务：阻塞接收
// 等着，没帧就挂起
xQueueReceive(s_frame_queue, &work_item, portMAX_DELAY);
```

> **为什么队列深度是 4？**
> 因为 DSP 处理一帧的时间必须小于 4 帧的缓冲区持续时间（40ms）。如果处理时间超过 40ms → 队列满了丢帧 → 丢帧不会导致系统崩溃，只是语音识别短暂卡顿——这是**音频系统的典型权衡**：允许偶尔丢帧，但不能让 CPU 卡死。

---

## 六、StreamBuffer — 变长数据传递

当传递的数据不是固定大小（而是变长的字节流），用队列需要每次都分配最大长度的项——浪费。**StreamBuffer** 是更好的选择。

```c
// 本项目在 OTA 中的使用（xvf_ota.c:183）
// 创建一个 16KB 的流缓冲
s_ota_stream = xStreamBufferCreate(16 * 1024, 1);
// 第二个参数 1 = 触发级别（trigger level），
// 即接收方至少等 1 字节才被唤醒

// CDC 数据到达回调（ISR上下文）：写入流缓冲
xStreamBufferSend(s_ota_stream, data, len, 0);  // timeout=0，不阻塞中断

// ota_flash 任务：等待并读取
size_t received = xStreamBufferReceive(s_ota_stream, buf, sizeof(buf),
                                       pdMS_TO_TICKS(2000));
```

| 场景 | 用 Queue | 用 StreamBuffer |
|------|:--------:|:---------------:|
| 固定长度的结构体 | ✅ 首选 | ❌ |
| 变长字节流（串口、CDC、音频裸数据） | ❌ 频繁分配 | ✅ 完美 |
| 多个同类型数据 | ✅ | ❌ 只支持 1 对 1 |

---

## 七、线程同步（IPC）方式对比

本项目实际使用的 IPC 方式涵盖了三种最常见的模式：

| IPC 方式 | 性能 | 特性 | 本项目用法 |
|---------|:----:|------|-----------|
| **Task Notification** | ⭐⭐⭐ | 最快的 1-to-1 通知，无需创建对象 | USB ISR → 唤醒 usb_spk_task / usb_mic_task |
| **Queue** | ⭐⭐ | 固定大小数据，多对一/一对多 | USB MIC → ds_dsp（音频帧传递） |
| **StreamBuffer** | ⭐⭐ | 变长字节流，一对一的流水线 | CDC ISR → ota_flash（OTA 固件流） |
| Semaphore / Mutex | ⭐⭐ | 信号/互斥 | 本项目未使用（被 Task Notification 和 spinlock 替代） |
| EventGroup | ⭐ | 多条件等待 | 本项目未使用 |

### 为什么本项目不用信号量？

很多 RTOS 教程花大篇幅讲信号量，但本项目实际**一个都没用**。它用了更高效的替代方案：

**① Task Notification 代替二进制信号量**：

```c
// ISR 端：USB 音频数据到达
bool tud_audio_rx_done_isr(...)
{
    // ... 读取 USB 数据到缓冲区 ...
    xTaskNotifyGive(s_uac_device->spk_task_handle);  // 发通知
    return true;
}

// 任务端：等通知
void usb_spk_task(void *pvParam)
{
    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  // 阻塞等通知
        // 通知来了 → 处理音频数据
    }
}
```

> Task Notification 比二进制信号量快约 **30%**，因为它使用任务自带的 TCB 字段，**不需要创建独立的内核对象**。每个任务天生有一个"通知状态"，直接利用即可。

**② spinlock 代替互斥锁**：

对于仅保护少量代码（如一个变量赋值）的临界区，`taskENTER_CRITICAL()`（关中断）比 Mutex 更轻量：

```c
// ds_dsp 和 usb_mic 共享一个 48k 输出 FIFO
portMUX_TYPE s_uac_out_mux = portMUX_INITIALIZER_UNLOCKED;

// ds_dsp 写入采样
taskENTER_CRITICAL(&s_uac_out_mux);
fifo_write(&s_48k_out, sample);
taskEXIT_CRITICAL(&s_uac_out_mux);
```

> **Mutex vs spinlock 的选择**：Mutex 是"等不到就阻塞"（适合长时间等待），spinlock 是"等不到就原地循环"（适合极短操作）。本项目临界区只有一行赋值，spinlock 更高效。

---

## 八、`vTaskDelay` vs `vTaskDelayUntil`

延迟是每个任务都会用到的功能。FreeRTOS 提供了两种方式：

### 相对延迟 `vTaskDelay`

**从当前时刻开始等 N 个 tick**。缺点是：如果任务执行时间有波动，周期会偏移。

```c
// 本项目中使用 vTaskDelay 的 6 个模式：
vTaskDelay(pdMS_TO_TICKS(1000));    // ① DoA/VAD 每秒轮询一次
vTaskDelay(pdMS_TO_TICKS(1));       // ② I2S 非阻塞轮询循环中让出 CPU
vTaskDelay(pdMS_TO_TICKS(5));       // ③ 扬声器静音填充每 5ms 一次
vTaskDelay(pdMS_TO_TICKS(300));     // ④ OTA 重启前留出串口输出的时间
vTaskDelay(pdMS_TO_TICKS(10));      // ⑤ I2C 通信失败后重试等待
vTaskDelay(pdMS_TO_TICKS(2000));    // ⑥ 启动后等待 2s 再读取 DoA
```

### 绝对延迟 `vTaskDelayUntil`

**指定一个绝对唤醒时刻**。任务运行时间波动不会影响周期稳定性——只要任务执行时间不超过周期本身。

```c
TickType_t xLastWakeTime = xTaskGetTickCount();

while (1) {
    // ... 每次执行这里的时间可能不同 ...
    do_mic_read();

    // 等"下一个 20ms 边界"，而不是"等 20ms"
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(20));
}
```

### 区别图解

```
相对延迟 vTaskDelay(20):
    等待 等待 等待 等待
A---|----|----|----|----|----|----|----|----
  ^t0  10ms 20ms                   ^t0+δ+20ms

    周期不是均匀的——因为任务 A 的执行时间不同

绝对延迟 vTaskDelayUntil(&last, 20):
    固定 20ms
A---|---|---|---|---|---|---|---|---
  ^t0      ^t0+20  ^t0+40  ^t0+60

    始终等"下一个 20ms 边界"
```

> 本项目中使用 `vTaskDelayUntil` 来**保证麦克风读取的精确周期**——音频采样需要稳定的定时节拍。

---

## 九、双核调度

ESP32-S3 有两个核心。FreeRTOS 的对称多处理（SMP）模式下，两个核心共享同一个就绪任务列表：

```c
// 项目中的核心分配策略
Core 0: 所有音频相关任务
  ├── usb_mic_task (prio 14)     ← 音频输入
  ├── usb_spk_task (prio 14)     ← 音频输出
  ├── ds_dsp       (prio 5)      ← 音频处理
  ├── spk_wdog     (prio 3)      ← 音频保活
  └── ota_flash    (prio 3)      ← 后台固件写入

Core 1: USB 协议栈 + 非音频任务
  ├── TinyUSB      (prio 15)     ← USB 协议栈（最高优先级）
  └── doa_vad      (prio 5)      ← 声源定位轮询
```

```mermaid
gantt
    title 双核典型运行时间线
    dateFormat  YYYY-MM-DD
    axisFormat  %L
    section Core 0
    usb_mic_task    :a1, 0, 5ms
    usb_spk_task    :a2, 5ms, 5ms
    ds_dsp          :a3, 10ms, 10ms
    spk_wdog        :a4, 20ms, 2ms

    section Core 1
    TinyUSB         :b1, 0, 2ms
    doa_vad         :b2, 2ms, 1ms
    TinyUSB         :b3, 3ms, 2ms
    doa_vad         :b4, 5ms, 1ms
```

> **设计原则**：将实时性要求高的音频 I/O 放在同一个核心上（Core 0），利用同一核心的任务优先级抢占保证延迟。USB 协议栈放在另一个核心，不干扰音频路径。两个核心间极少共享数据，避免了复杂的跨核同步。

---

## 十、栈大小与调试

### 10.1 栈溢出的后果

任务栈溢出是 RTOS 中最隐蔽的灾难——不会编译报错，运行时也不会立即崩溃。它悄悄覆盖相邻内存，在某次函数调用时突然触发非法指令 → 看门狗复位。

```c
// 任务栈在内存中的布局
┌──────────────────────┤ 任务栈 ├──────────────────────┐
│  局部变量 │  函数调用栈 │  ...  │  空闲区域  │  栈哨兵  │
└──────────────────────────────────────────────────────┘
                                    ↑ 触到哨兵 → FreeRTOS 报告
```

### 10.2 FreeRTOS 栈溢出检查

```c
// sdkconfig 相关配置
CONFIG_FREERTOS_CHECK_STACKOVERFLOW_CANARY=y    // 栈末尾写入已知值
CONFIG_FREERTOS_WATCHPOINT_END_OF_STACK=y       // 硬件断点监控栈末尾
```

两种机制：
- **栈哨兵（Canary）**：在栈末尾写入一个已知值，每次任务切换时检查是否被覆盖
- **硬件断点（Watchpoint）**：使用 ESP32-S3 的硬件断点监控栈边界，触碰即触发异常——更精确

### 10.3 查看各任务的实际栈使用

ESP-IDF 提供了在运行时查询任务栈使用情况的 API：

```c
// 在代码中随时查询
UBaseType_t high_water_mark = uxTaskGetStackHighWaterMark(task_handle);
// 返回"该任务历史上还剩的最小可用栈空间"（单位：字）

// 在串口控制台调试（本项目的 CDC 命令中注册了此功能）
UBaseType_t stack_free = uxTaskGetStackHighWaterMark2(NULL);
// NULL = 查询当前正在运行的任务自身
```

> **`uxTaskGetStackHighWaterMark()` 是调试栈大小最常用的函数**。如果某任务 high water mark 接近 0，说明它的栈应该加大。本项目 7 个任务中有 6 个栈为 4096，唯一特殊的 `ota_flash` 用了 8192——因为它在栈上分配了页大小的临时缓冲区用于 Flash 写入。

### 10.4 ESP-IDF 增强：UXTASK_GET_STACK_INFO

ESP-IDF 在 FreeRTOS 之上补充了更详细的栈查询：

```c
// 获取更详细的栈信息
TaskSnapshot_t snapshot;
vTaskGetSnapshot(task_handle, &snapshot);
// snapshot 中包含: pxTopOfStack, pxEndOfStack, pxHighWaterMark
```

> 在串口终端中输入 `info tasks`（或 ESP-IDF 的类似命令）可以在运行时打印所有任务的信息，包括名称、优先级、状态、核心、栈使用峰值。

---

## 十一、任务优先级设计原则

### 通用原则

| 原则 | 说明 | 反模式 |
|------|------|--------|
| **实时性越强，优先级越高** | I2S 音频 DMA 链→高；日志写 SPIFFS→低 | 全设成一样高 = 等于没设 |
| **阻塞多的给高优先级** | 它大部分时间在等，不占用 CPU | 计算密集型给最高 = 其他任务挨饿 |
| **ISR 只做最少的活** | ISR 里只放 `xQueueSendFromISR` / `xTaskNotifyGive` | ISR 里 printf / malloc = 系统崩溃 |
| **相等优先级 = 时间片轮转** | 互相协作，谁都不饿死谁 | 以为不相等才公平 |

### 常见误区

> **误区 1：优先级越高越好**
> 错。优先级越高，剥夺其他任务 CPU 的能力越强。给所有任务都是最高优先级 = 纯轮转，实时性反而没有保证。
>
> **误区 2：任务数越少越好**
> 错。合理拆分为多个独立任务反而更清晰、更容易调试。崩溃的一个任务不会影响其他任务。
>
> **误区 3：不在两个核上放相同优先级**
> 错。FreeRTOS SMP 能正确处理——双核各自挑最高优先级的就绪任务运行。

### 本项目验证的设计规律

从本项目的 7 个任务中可以总结出一个**可复用的优先级模板**：

| 层级 | 优先级 | 典型任务 | 数量范围 |
|------|:------:|---------|:-------:|
| 高（系统关键） | 14~15 | 音频 I/O、USB 栈 | 2~3 |
| 中（数据处理） | 4~6 | DSP、按键、通信协议 | 1~3 |
| 低（后台任务） | 1~3 | 日志、OTA、看门狗 | 1~3 |

---

## 十二、FreeRTOS 调试与日志

ESP-IDF 对 FreeRTOS 做了一些重要的扩展：

| 功能 | API / 配置 | 用途 |
|------|-----------|------|
| 运行时统计 | `vTaskList()` / `vTaskGetRunTimeStats()` | 打印各任务 CPU 占用率 |
| 栈使用 | `uxTaskGetStackHighWaterMark()` | 检查栈余量 |
| 任务快照 | `vTaskGetSnapshot()` | 获取任务完整状态 |
| 内核可视化 | `CONFIG_FREERTOS_USE_TRACE_FACILITY=y` | 使能上述统计功能 |
| OpenOCD 感知 | `CONFIG_FREERTOS_DEBUG_OCDAWARE=y` | 调试器知道当前是哪个任务在跑 |
| 栈溢出检测 | `CONFIG_FREERTOS_CHECK_STACKOVERFLOW_CANARY=y` | 自动检测栈溢出 |
| 任务函数包装 | `CONFIG_FREERTOS_TASK_FUNCTION_WRAPPER=y` | 任务意外返回时捕获并报错 |

> 上述所有功能都在本项目的 `sdkconfig` 中启用。

---

## 十三、FreeRTOS 内存管理 — heap_caps_malloc 与"内存在哪"的烦恼

### 13.1 FreeRTOS 的五种堆模型

FreeRTOS 内核内存分配有 5 种可选方案（`heap_1.c` ~ `heap_5.c`）：

| 方案 | 特点 | 适用于 |
|:----:|------|--------|
| `heap_1` | 只能分配不能释放，最简单 | 最小系统，对象全在创建时一次性配好 |
| `heap_2` | 可分配可释放，但不合并相邻空闲块 | 早期 FreeRTOS 项目 |
| `heap_3` | 包装 `malloc/free`，线程安全 | 有标准 C `malloc` 的平台 |
| `heap_4` | 可分配释放 + **空闲块合并** | **最常见**，避免内存碎片 |
| `heap_5` | `heap_4` + 多块非连续内存 | 同时有片内 SRAM *和* PSRAM 的系统 |

> **ESP-IDF 的默认方案是 `heap_4`** ——本项目也不例外。ESP-IDF 在此基础上增加了一个重要层次：**能力标签 (caps)**。

### 13.2 问题：ESP32-S3 不止一块内存

回顾第 2 课的存储章节：ESP32-S3 有 512KB 片内 SRAM 和 8MB 片外 PSRAM。**标准 `malloc` 只知道从片内 SRAM 分配**——如果你的模型文件 3.7MB，`malloc(3700000)` 直接返回 NULL。

```c
// 普通 malloc → 只从片内 SRAM 分配
void *buf = malloc(3700000);  // → NULL！512KB 装不下
```

这就是 `heap_caps_malloc()` 的用武之地——ESP-IDF 扩展，**让你指定分配在哪类内存上**。

```c
void *heap_caps_malloc(size_t size, uint32_t caps);

// 常用 caps ：
MALLOC_CAP_INTERNAL    // 片内 SRAM（快，少）
MALLOC_CAP_SPIRAM      // 片外 PSRAM（大，慢）
MALLOC_CAP_8BIT        // 必须支持字节寻址（PSRAM 支持）
MALLOC_CAP_DMA         // 必须支持 DMA 访问
```

### 13.3 本项目的实战模式

这是在 `xvf_multinet.c` 中加载语音识别模型的真实分配逻辑：

```c
// 第 1 步：优先用 PSRAM（容量充足）
s_detect_buf = heap_caps_malloc(
    s_chunk_size * sizeof(int16_t),         // ~65KB
    MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT     // PSRAM + 字节寻址
);

// 第 2 步：PSRAM 不可用时，fallback 到片内 SRAM
if (!s_detect_buf) {
    s_detect_buf = malloc(s_chunk_size * sizeof(int16_t));
}

// 第 3 步：还不成功才报错
if (!s_detect_buf) {
    ESP_LOGE(TAG, "No memory for detect buffer");
    return ESP_ERR_NO_MEM;
}
```

> **关键是分层 fallback：PSRAM → 片内 SRAM → 报错**。不是直接报 `oom`。

### 13.4 堆"从哪里来"的可视化

```mermaid
flowchart TD
    API["malloc / free<br/>heap_caps_malloc / heap_caps_free<br/>calloc"]
    API --> LAYER[ESP-IDF 堆分配层<br/>根据 caps 参数路由]
    LAYER -->|caps=INTERNAL| INT["片内 SRAM 堆<br/>~362KB"]
    LAYER -->|caps=SPIRAM| EXT["片外 PSRAM 堆<br/>~8MB"]
    LAYER -->|caps混用| BOTH["两处都尝试"]

    INT -->|"FreeRTOS heap_4<br/>空闲块合并"| SRAM[片内 SRAM 物理内存]
    EXT --> PSRAM[片外 PSRAM 物理内存]
```

### 13.5 关键配置值

从本项目的 `sdkconfig`：

| 配置 | 值 | 含义 |
|------|:---:|------|
| `CONFIG_SPIRAM_USE_MALLOC` | y | `malloc()` 可自动从 PSRAM 分配（透明化） |
| `CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL` | 16384 | 小于 16KB 的分配优先走片内 SRAM |
| `CONFIG_SPIRAM_MALLOC_RESERVE_INTERNAL` | 32768 | 始终保留 32KB 片内 SRAM 给内核 |
| `CONFIG_SPIRAM_ALLOW_STACK_EXTERNAL_MEMORY` | y | 任务栈可分配到 PSRAM |
| `CONFIG_FREERTOS_TASK_CREATE_ALLOW_EXT_MEM` | y | 任务 TCB 和栈可在 PSRAM 分配 |

> **设计意图**：小分配（<16KB）走快内存，大分配（模型文件、DMA缓冲）走大内存。这 32KB 保留空间确保即使 PSRAM 被大量分配，内核关键操作也有片内 SRAM 可用。

### 13.6 运行时查内存

```c
// 查看片内 SRAM 剩余
size_t free_internal = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);

// 查看 PSRAM 剩余
size_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);

// 本项目在 OTA 开始前检查（xvf_ota.c:172）
ESP_LOGI(TAG, "Free mem: internal=%lu, psram=%lu",
         (unsigned long)heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
         (unsigned long)heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
```

---

## 十四、EventGroup — 等"多个条件同时满足"

### 14.1 概念

队列是等一条消息。信号量是等一个信号。**EventGroup 是等一组条件中某几个同时满足**。

```mermaid
flowchart LR
    subgraph TASK[任务 — 等待]
        WAIT[xEventGroupWaitBits<br/>等 bit0 AND bit1]
    end

    subgraph EVT[事件组 32-bit]
        B0[bit0: 文件系统初始化完成]
        B1[bit1: WiFi 连接成功]
        B2[bit2: NTP 时间同步完成]
    end

    subgraph ISR1[TASK_A 设置]
        S1["做完初始化 → xEventGroupSetBits(evt, BIT0)"]
    end
    subgraph ISR2[TASK_B 设置]
        S2["WiFi 连上 → xEventGroupSetBits(evt, BIT1)"]
    end

    S1 --> B0
    S2 --> B1
    B0 & B1 --> WAIT
```

### 14.2 API 概览

```c
// 创建事件组
EventGroupHandle_t evt = xEventGroupCreate();

// 设置 bit（从任务）
xEventGroupSetBits(evt, BIT0);
// 设置 bit（从 ISR）
xEventGroupSetBitsFromISR(evt, BIT0, &xHigherPriorityTaskWoken);

// 等待 bit（多个选项）
EventBits_t bits = xEventGroupWaitBits(
    evt,
    BIT0 | BIT1,     // 等 bit0 和 bit1
    pdTRUE,           // 消费模式：读完后清空
    pdTRUE,           // 等 ALL bits（AND）；pdFALSE = 等 ANY bits（OR）
    portMAX_DELAY     // 永久等待
);
```

### 14.3 AND vs OR 逻辑

| 模式 | 含义 | 例子 |
|:----:|------|------|
| `pdTRUE` (AND) | 所有指定位都置 1 时才返回 | 系统启动：等 SPIFFS **且** WiFi **且** PSRAM 都就绪 |
| `pdFALSE` (OR) | 任意位被置 1 就返回 | 错误处理：等按键 **或** 网络断开 **或** 超时中任一触发 |

> **AND 模式是 EventGroup 最独特的价值**——其他 IPC 机制做不到"同时等多个条件都满足才继续"。

### 14.4 典型场景：启动序列同步

```c
// 三个后台任务各自做初始化，做完后"亮灯"
void wifi_init_task(void *pv) {
    wifi_connect();
    xEventGroupSetBits(g_boot_evt, WIFI_READY_BIT);  // bit 0 = 1
    vTaskDelete(NULL);
}

void filesystem_init_task(void *pv) {
    spiffs_mount();
    xEventGroupSetBits(g_boot_evt, FS_READY_BIT);    // bit 1 = 1
    vTaskDelete(NULL);
}

void model_init_task(void *pv) {
    load_model_into_psram();
    xEventGroupSetBits(g_boot_evt, MODEL_READY_BIT);  // bit 2 = 1
    vTaskDelete(NULL);
}

// 主任务等全部三个就绪
void main_init_task(void *pv) {
    xEventGroupWaitBits(g_boot_evt, 
                        WIFI_READY_BIT | FS_READY_BIT | MODEL_READY_BIT,
                        pdTRUE,   // 清空已消费的 bits
                        pdTRUE,   // AND — 三个都要
                        pdMS_TO_TICKS(30000));  // 最多等 30 秒
        
    ESP_LOGI("MAIN", "All subsystems ready, starting main loop");
}
```

> 本项目**没有用 EventGroup**——它的初始化顺序是严格的串行依赖，不需要并行等。但你在开发稍复杂的系统时，EventGroup 是第一个要考虑的"多源同步"工具。

---

## 十五、软件定时器（Software Timer）

### 15.1 概念：不用独立任务就能实现周期操作

如果一个任务只做"每 X ms 检查一次某件事"，那就浪费了一个任务栈和 TCB。**软件定时器**让你注册一个回调函数，由 FreeRTOS 内核自动周期调用。

```c
// 不需要开独立任务 + 写 while(1) + vTaskDelay
// 三行就搞定！
TimerHandle_t timer = xTimerCreate(
    "myTimer",                       // 名称
    pdMS_TO_TICKS(1000),             // 周期：1000ms
    pdTRUE,                          // pdTRUE=自动重载, pdFALSE=一次性
    (void *)0,                       // 传入回调的参数
    myTimerCallback                  // 回调函数
);
xTimerStart(timer, 0);
```

### 15.2 回调在哪里执行？

```mermaid
flowchart LR
    TICK["Tick ISR<br/>每 1ms"] -->|"发现定时器到期"| QUEUE["定时器命令队列"]
    QUEUE -->|"添加到"| TMR["Timer 服务任务<br/>prio 1, stack 2048"]
    TMR -->|"串行调用"| CB["myTimerCallback()"]
```

> **回调不在 Tick ISR 中执行，也不在你的任务中执行——它在"Timer 服务任务"中执行！** 这是关键：你的回调必须快，因为**所有定时器共享这一个任务**，一个慢回调会堵住其他所有定时器。

### 15.3 本项目的配置

```c
CONFIG_FREERTOS_USE_TIMERS=y
CONFIG_FREERTOS_TIMER_TASK_PRIORITY=1        // 低优先级（不影响音频 I/O）
CONFIG_FREERTOS_TIMER_TASK_STACK_DEPTH=2048  // 2KB 栈
CONFIG_FREERTOS_TIMER_QUEUE_LENGTH=10        // 最多排队 10 个定时器事件
```

### 15.4 何时用 Timer vs 独立 Task？

| 场景 | 用 Timer | 用独立 Task |
|------|:--:|:--:|
| 每 N ms 做一件很短的事（翻转 LED、看门狗喂狗） | ✅ | ❌ 重了 |
| 操作涉及阻塞等待（等队列、等 I2C 应答） | ❌ 会堵住其他 Timer | ✅ |
| 需要维护状态变量跨周期（计数器、滤波值） | ❌ 状态管理在回调里难做 | ✅ |
| 需要被暂停/恢复/动态改周期 | ✅ 内置 API 支持 | ❌ 需要自己实现 |

### 15.5 一次性定时器：延时执行

```c
// "3 秒后执行一次回调，然后自动删除"
TimerHandle_t once = xTimerCreate("once", pdMS_TO_TICKS(3000), pdFALSE, 0, callback);
xTimerStart(once, 0);
// 3s 后 callback 被调用 → 定时器自动停止 → 只需手动 xTimerDelete()
```

> 本项目未使用软件定时器——它需要精确周期（`vTaskDelayUntil`）和阻塞等待（I2C 通信），适合独立任务而非定时器回调。

---

## 十六、Task 的实现细节与设计哲学

这部分不是教 API，而是让你理解 **xTaskCreate 调用时，FreeRTOS 内核在背后做了什么**。理解这些能让你的任务设计更有把握。

### 16.1 TCB（Task Control Block）— 任务的内核身份证

每个任务的 TCB = 一个 C 结构体，约 100+ 字节（精简版）：

```c
// 简化版 TCB 结构（仅核心字段）
typedef struct tskTaskControlBlock {
    volatile StackType_t *pxTopOfStack;   // ① 当前栈顶指针（上下文切换的关键！）
    ListItem_t            xStateListItem; // ② 挂在"就绪/阻塞/挂起"链表上
    ListItem_t            xEventListItem; // ③ 挂在"等待事件"链表上（如有）
    UBaseType_t           uxPriority;     // ④ 当前优先级
    StackType_t           *pxStack;       // ⑤ 栈底部（起始地址）
    char                  pcTaskName[16]; // ⑥ 任务名（调试用）
    TaskHandle_t          xTaskHandle;    // ⑦ 自引用句柄
    // ... SMP 扩展字段（核心亲和性等）...
} tskTCB;
```

```mermaid
flowchart TD
    subgraph TCB["TCB 在内存中的关系"]
        direction LR
        STACK["任务专用栈
        ┌─────────────┐
        │ 局部变量     │ ← 栈生长方向
        │ 函数调用帧   │
        │ 上下文快照   │
        │ (寄存器保存)  │
        ├─────────────┤
        │ 空闲区域     │
        │ CANARY 哨兵  │
        └─────────────┘"]
        TCB_STRUCT["TCB 结构体
        pxTopOfStack ──→ 栈顶
        pxStack ───────→ 栈底
        uxPriority=5
        pcTaskName='ds_dsp'
        链表节点 ─────→ 就绪/阻塞链表"]
    end
```

### 16.2 xTaskCreate 内部：到底做了什么

```c
// xTaskCreate 的精简内核流程
// （实际比这复杂得多，仅示核心步骤）

BaseType_t xTaskCreate(...)
{
    // ① 分配 TCB（如 CONFIG_FREERTOS_TASK_CREATE_ALLOW_EXT_MEM=y，可能在 PSRAM）
    TCB_t *pxNewTCB = pvPortMalloc(sizeof(TCB_t));

    // ② 分配任务栈（StackType_t 是 32-bit 字）
    //    usStackDepth=4096 → 4096 × 4 = 16384 字节
    StackType_t *pxStack = pvPortMalloc(usStackDepth * sizeof(StackType_t));

    // ③ 初始化栈：在栈顶写入"假上下文"——假装任务被某个 ISR 打断过
    //    这样当调度器第一次"恢复"这个任务时，就会跳到任务函数的入口
    pxNewTCB->pxTopOfStack = pxPortInitialiseStack(
        pxStack,             // 栈底
        pvTaskCode,          // 任务函数（要"跳"过去）
        pvParameters         // 参数
    );

    // ④ 填充 TCB 元数据
    pxNewTCB->uxPriority = uxPriority;
    strncpy(pxNewTCB->pcTaskName, pcName, 16);
    pxNewTCB->pxStack = pxStack;

    // ⑤ 把 TCB 挂到对应优先级的就绪链表中
    prvAddTaskToReadyList(pxNewTCB);

    // ⑥ 如果新任务优先级 > 当前任务 → 触发 PendSV 上下文切换
    if (uxPriority > pxCurrentTCB->uxPriority) {
        portYIELD();  // 触发 PendSV
    }

    return pdPASS;
}
```

> **关键洞见**：创建任务时**它还没有运行过**。内核在栈上伪造了一个"刚被中断打断"的状态——包含初始程序计数器（PC = 任务函数地址）、初始栈指针、初始状态寄存器等。当调度器通过 PendSV "恢复"这个任务时，CPU 就"回到"了任务函数的入口，开始执行。

### 16.3 PendSV — 上下文切换的幕后之手

上下文切换不是通过函数调用来做的——函数调用需要主动调用。**必须有一个不可抗拒的机制强制切走当前任务**。ARM Cortex-M 的答案是一个特殊异常：**PendSV（可挂起的系统调用异常）**。

```mermaid
sequenceDiagram
    participant TICK as SysTick ISR
    participant HW as 硬件(NVIC)
    participant ISR as PendSV Handler
    participant OLD as 旧任务
    participant NEW as 新任务

    Note over OLD: 正在运行

    TICK->>HW: ① Tick 中断到达，发现应切换
    TICK->>HW: ② 触发 PendSV 异常（不直接切换！）
    TICK-->>OLD: ③ SysTick ISR 返回

    Note over HW: PendSV 优先级最低<br/>等所有其他中断完成

    HW->>ISR: ④ 所有中断完成，PendSV 获得执行权
    ISR->>ISR: ⑤ 保存旧任务上下文到 old_tcb->pxTopOfStack
    ISR->>ISR: ⑥ 从 new_tcb->pxTopOfStack 恢复新任务上下文
    ISR-->>NEW: ⑦ PendSV 返回 → 硬件自动弹出栈帧 → 新任务开始执行
```

> **PendSV 的设计精髓**：
> - 它是**软件可触发**的（`portYIELD()` → 置位 PendSV 挂起位）
> - 它的优先级被设为**最低**（所有 ISR 都比它高）
> - 结果：ISR 首先完成自己的紧急工作，然后**在 ISR 返回时顺带做任务切换**——不会阻塞硬件中断的响应
>
> 这就是为什么你在 ISR 结尾调用 `portYIELD_FROM_ISR()`：它不是在 ISR 内部立刻切换，而是设置一个"记得切换"的标志。PendSV 会在 ISR 完全退出后才处理。

### 16.4 就绪链表 — 调度器的"任务目录"

每个优先级对应一个就绪链表——调度器不需要遍历所有任务来找最高优先级：

```c
// FreeRTOS 内核中的就绪链表结构（简化）
// 假设 configMAX_PRIORITIES = 25（本项目的值）

List_t pxReadyTasksLists[25];  // pxReadyTasksLists[14] = 所有 prio 14 的就绪任务

// 还有两个特殊链表：
List_t xDelayedTaskList1;      // 正在等待延时过期的任务
List_t xDelayedTaskList2;      // 等待队列/信号量的任务
List_t xPendingReadyList;      // 刚从 ISR 中释放的待就绪任务
```

```
调度的第一步：找到最高非空优先级
    pxReadyTasksLists[24] → 空
    pxReadyTasksLists[15] → [TinyUSB]       ← 24-bit 位图中 bit 15=1
    pxReadyTasksLists[14] → [usb_mic → usb_spk]  ← bit 14=1
    ...
```

> 为了加速查找，FreeRTOS 使用了一个 **32-bit 位图**：`uxTopReadyPriority` 的某个 bit = 1 表示该优先级有就绪任务。`__builtin_clz()` 一条 CPU 指令就能定位最高非零 bit。这意味着**找到下一个要跑的任务是 O(1) 操作**——不管系统有多少个任务。

### 16.5 栈帧的真相 — 任务"睡着"时长什么样

当一个任务不运行时，它的栈顶保存着一个完整的 CPU 上下文快照：

```
低地址（栈底）
┌──────────────────┐
│    任务栈内容     │  ← 局部变量、函数调用帧
│    (实际使用)    │
├──────────────────┤  ← pxTopOfStack 指向这里
│    R4            │  ← 被调用者保存寄存器（自动）
│    R5            │
│    R6            │
│    R7            │
│    R8            │
│    R9            │
│    R10           │
│    R11           │
│    R0 (返回值)    │
│    R1            │
│    R2            │
│    R3            │
│    R12           │
│    LR (返回地址)  │  ← 函数调用时的链接寄存器
│    PC (下一条指令)│  ← 程序计数器
│    xPSR (状态)   │  ← CPU 状态寄存器
├──────────────────┤  ← 栈顶（高地址）
│   未使用的空间    │
│   ...            │
│   CANARY (哨兵)  │  ← 溢出检测用的魔法值
└──────────────────┘
高地址（栈顶）
```

> **"上下文切换"的本质**：把当前 CPU 的 16 个核心寄存器值压入当前任务栈 → 切换 `pxCurrentTCB` → 从新任务栈弹出寄存器值 → `BX LR` 跳转到新任务最后停下的位置。这一切由 PendSV Handler 完成，约 **<1 μs**。

### 16.6 设计哲学：为什么任务不只做一次就退出？

回到 FreeRTOS 的设计精神。任务不是函数调用——它更像一个**独立的线程**。

```c
// ❌ 反模式：任务是"一次性执行"
void bad_task(void *pv) {
    do_something();
    return;  // 返回后 TCB 变成僵尸——除非显式 vTaskDelete(NULL)
}

// ✅ 正确模式：任务是"永恒的循环"  
void good_task(void *pv) {
    // 只运行一次的初始化
    init();

    while (1) {
        // ① 等事件（阻塞，让出 CPU）
        wait_for_event();
        // ② 处理事件
        handle();
        // ③ 回到 ① 等待下一个事件
        //    这期间 CPU 可能去跑其他任务
    }
}
```

> **任务 = 事件循环**。每个任务都是"等着做某件事"的永久循环。不做任何事的时候，任务处于 Blocked 状态——不消耗 CPU。这就是 RTOS 比 Super Loop 高效的根源：**不做事的时候，CPU 完全不消耗在你身上**。

### 16.7 任务删除与清理

任务可以删除自己（`vTaskDelete(NULL)`），或被其他任务删除。关键问题：**谁负责释放任务的栈和 TCB？**

```c
// 删除自己
void one_shot_task(void *pv) {
    do_something_once();
    vTaskDelete(NULL);   // ① 标记为删除，但栈在 *这一刻* 还被占用
    // ② 永远不会执行到这里——vTaskDelete 不返回
}

// 删除其他任务
void manager_task(void *pv) {
    // ...
    vTaskDelete(worker_handle);
    // worker 的栈和 TCB 由空闲任务（Idle Task）负责回收
}
```

> FreeRTOS 中，被删除任务的资源（栈、TCB）**由 Idle Task 回收**，不是被调用 `vTaskDelete` 的任务回收。这保证了即使任务是删除自己——资源也能被正确释放。ESP-IDF 还提供 `CONFIG_FREERTOS_TASK_FUNCTION_WRAPPER=y`，在任务意外返回时自动调用 `vTaskDelete(NULL)`，防止僵尸 TCB 泄漏。

---

## 课后思考题

1. 如果你这个项目只有一个核心（Core 1 不存在），TinyUSB 任务（prio 15）和 usb_spk_task（prio 14）在同一个核上——usb_spk_task 会饿死吗？
2. 什么样的任务适合用 `vTaskDelayUntil` 而不是 `vTaskDelay`？
3. 如果 `CONFIG_FREERTOS_HZ` 改为 100（10ms tick），对系统中哪个任务影响最大？
4. 为什么可以用 Task Notification 代替 Binary Semaphore？性能优势来自哪里？
5. 本项目 7 个任务中，为什么 `ota_flash` 的栈（8192）是其他任务的两倍？
6. `heap_caps_malloc(size, MALLOC_CAP_SPIRAM)` 失败后为什么还要 fallback 到 `malloc()`？两者有什么本质区别？
7. 软件定时器的回调函数中，调用 `vTaskDelay` 会发生什么？为什么？
8. PendSV 为什么优先级必须设为最低？如果设得比 SysTick 还高会怎样？
