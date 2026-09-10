---
created: 2026-08-20
modified: 2026-08-20
tags:
  - type/article
  - topic/webrtc
  - topic/robotics
  - lang/cpp
---
# OpenTera WebRTC：机器人遥操作的音视频流框架

> **项目地址**: [introlab/opentera-webrtc](https://github.com/introlab/opentera-webrtc)（注：非 `-core` 后缀）
> **机构**: IntRoLab, Université de Sherbrooke（加拿大舍布鲁克大学智能机器人实验室）
> **版本**: 1.2.6（最后提交 2025-04）
> **授权**: Apache 2.0（注意：默认构建的 libwebrtc 含非自由编解码器）
> **语言**: C++14 核心 + Python 3 绑定（pybind11）+ JavaScript
> **依赖**: libwebrtc（Google 原生）、GStreamer（可选）、IXWebSocket、OpenCV、pybind11、nlohmann/json

---

## 一、项目定位：为什么不用浏览器那套？

WebRTC 是浏览器里的实时音视频标准，但**机器人端不能只依赖浏览器** —— 需要细粒度控制码流、编解码器选择、以及嵌入式平台（Jetson）上的硬件加速。OpenTera WebRTC 就是为这个场景做的：

为 [OpenTera](https://github.com/introlab/opentera)（远程康复微服务架构）提供**原生 WebRTC 能力**，让机器人能够与浏览器/其他客户端建立低延迟的 P2P 音视频与数据通道。

交付五大件：

| 组件 | 语言 | 作用 |
|------|------|------|
| **signaling-server** | Python (aiohttp) | 信令服务器，可独立运行或嵌入 OpenTera 微服务 |
| **opentera-webrtc-native-client** | C++14 | libwebrtc 原生封装，机器人端核心库 |
| **OpenteraWebrtcNativeGStreamer** | C++ | GStreamer 编解码工厂，软/硬件编码器 |
| **Python 绑定** | pybind11 | 让 Python 直接调用原生客户端 |
| **opentera-webrtc-web-client** | JavaScript | 浏览器端库，用于遥操作界面 |

---

## 二、整体架构

```
┌──────────────────────────────────────────────────────────────┐
│                    信令服务器 (Python/aiohttp)                  │
│  ┌────────────────────────────────────────────────────────┐  │
│  │ WebSocket /signaling  ← 事件路由（join-room, call-peer） │  │
│  │ HTTP   /iceservers    ← STUN/TURN 配置下发              │  │
│  │ RoomManager           ← 房间/成员/名称映射               │  │
│  │ WebSocketClientManager ← 连接池、消息收发                │  │
│  └────────────────────────────────────────────────────────┘  │
└───────────▲──────────────────────────▲───────────────────────┘
            │ WebSocket（仅信令，JSON）  │
            │                          │
   ┌────────┴────────┐        ┌────────┴─────────┐
   │  机器人端        │        │   浏览器端        │
   │ Native Client   │◄──────►│  Web Client      │
   │ (C++/Python)    │  P2P   │  (JavaScript)    │
   │                 │ WebRTC │                  │
   │ 音视频流 + 数据通道      │  RTCPeerConnection│
   └─────────────────┘  直连   └──────────────────┘
```

**核心设计：信令与媒体彻底分离。**

- **信令**走 WebSocket 中心服务器（房间发现、会话协商、SDP/ICE 转发）
- **媒体**一旦协商完成就走 P2P 直连（不经服务器），保证低延迟

这是 WebRTC 的标准范式，但本项目的价值在于把两端（原生 + 浏览器）统一到**同一套信令协议**下。

---

## 三、信令协议（最值得学习的部分）

协议版本：`PROTOCOL_VERSION = 2`（服务端与客户端两端硬编码校验）

### 3.1 消息格式

```json
{ "event": "事件名", "data": { ... } }
```

### 3.2 客户端 → 服务器

| 事件 | data | 说明 |
|------|------|------|
| `join-room` | `name, data, room, password, protocolVersion` | 加入房间（含认证与版本校验） |
| `send-ice-candidate` | `toId, candidate{sdpMid, sdpMLineIndex, candidate}` | 转发 ICE 候选 |
| `call-peer` | `toId, offer{sdp, type:"offer"}` | 向指定 peer 发起呼叫 |
| `make-peer-call-answer` | `toId, answer{sdp, type:"answer"}` | 应答（不带 answer 字段即拒接） |
| `call-all` | — | 呼叫房间内所有人 |
| `call-ids` | `[id, ...]` | 呼叫指定 id 列表 |
| `close-all-room-peer-connections` | — | 请求全员断开 |

### 3.3 服务器 → 客户端

| 事件 | data | 说明 |
|------|------|------|
| `join-room-answer` | `sessionId` 或 `""` | 空串表示密码错误/版本不符 |
| `room-clients` | `[{id, name, data}, ...]` | 房间成员变化广播 |
| `peer-call-received` | `fromId, offer{sdp, type}` | 收到呼叫 |
| `peer-call-answer-received` | `fromId, answer` | 收到应答 |
| `ice-candidate-received` | `fromId, candidate` | 收到 ICE 候选 |
| `make-peer-call` | `[id, ...]` | 服务器指示主动呼叫这些 id（**避免呼叫风暴**） |
| `close-all-peer-connections-request-received` | — | 全员断开通知 |

### 3.4 建立连接的完整时序

```
机器人端A                信令服务器                 浏览器端B
   │                        │                        │
   │──join-room(name,room)─►│                        │
   │◄─join-room-answer(id)──│                        │
   │                        │◄──join-room────────────│
   │◄──room-clients([A,B])──│───room-clients([A,B])─►│
   │                        │                        │
   │──call-ids([B])────────►│                        │
   │                        │──make-peer-call([B])──►│  （服务器分发"该调用谁"）
   │                        │                        │
   │     ══════ WebRTC 协商（SDP offer/answer + ICE）══════
   │──call-peer(B, offer)──►│──peer-call-received───►│
   │                        │                        │
   │                        │◄─make-peer-call-answer─│
   │◄─peer-call-answer-received─────────────────────│
   │                        │                        │
   │──send-ice-candidate───►│──ice-candidate-received►│
   │◄─ice-candidate-received│◄─send-ice-candidate────│
   │                        │                        │
   │◄════════ P2P 媒体流直连（不再经过服务器）════════►│
```

**关键洞察**：`make-peer-call` 事件由服务器主动下发，是为了避免 N 个客户端同时向 M 个 peer 发起呼叫产生的连接风暴 —— 服务器用 `itertools.combinations(ids, 2)` 计算出**不重复的呼叫对**并分派。

### 3.5 服务器实现要点

**`room_manager.py`** — 四个字典 + 一把锁：

```python
_room_by_id        # client_id → room_name
_ids_by_room       # room_name → [client_id, ...]
_client_name_by_id # client_id → 显示名
_client_datum_by_id# client_id → 自定义数据
_lock = asyncio.Lock()  # 延迟创建（首次访问时）
```

> 设计细节：`asyncio.Lock()` 必须在事件循环内创建，所以用 `_create_lock_if_none()` 懒初始化 —— 这是 asyncio 编程的常见坑。

**`signaling_server.py`** — 超时与保活策略：

| 常量 | 值 | 作用 |
|------|-----|------|
| `PING_INTERVAL_S` | 10 | WebSocket 心跳间隔 |
| `INACTIVE_DELAY_S` | 5 | 连接后未加入房间则断开 |
| `DISCONNECT_DELAY_S` | 1 | 认证失败后延迟断开（防止时序问题） |

**安全机制**：
- `password` 全局口令校验（`join-room-answer` 返回空串表示失败）
- `protocolVersion` 必须匹配，否则拒绝
- CORS 中间件设置了 `Cross-Origin-Opener-Policy: same-origin` 等头（WebRTC 在浏览器要求跨域隔离才能用 `SharedArrayBuffer` 等能力）
- `/iceservers` 端点需 `Authorization` 头认证
- `--follow_symlinks` 明确标注为 SECURITY RISK

---

## 四、原生客户端（C++ 核心）

### 4.1 类层次

```
SignalingClient (抽象)
  └── WebSocketSignalingClient   ← IXWebSocket 实现

WebrtcClient (抽象基类)
  ├── StreamClient          ← 音视频流（VideoSource/AudioSource → 远端）
  └── DataChannelClient     ← 数据通道（任意字符串/二进制）
            └── (JS 侧还有 StreamDataChannelClient 混合体)
```

两者通过虚函数 `createPeerConnectionHandler()` 实现多态：

```cpp
virtual std::unique_ptr<PeerConnectionHandler>
    createPeerConnectionHandler(const std::string& id,
                                const Client& peerClient,
                                bool isCaller) = 0;
```

### 4.2 线程模型（重点）

`WebrtcClient` 内部管理 **4 个线程**：

```cpp
std::unique_ptr<rtc::Thread> m_internalClientThread;  // 状态主线程
std::unique_ptr<rtc::Thread> m_networkThread;         // 网络 I/O
std::unique_ptr<rtc::Thread> m_workerThread;          // 编码/处理
std::unique_ptr<rtc::Thread> m_signalingThread;       // 信令
```

对外 API 全部通过两个助手上线程：

```cpp
// 同步调用：取状态（阻塞当前线程，等内部线程返回）
inline bool WebrtcClient::isConnected() {
    return callSync(m_internalClientThread.get(),
                    [this]() { return m_signalingClient->isConnected(); });
}

// 异步调用：触发动作/回调
void WebrtcClient::log(const std::string& message) {
    callAsync(m_internalClientThread.get(), [=]() {
        if (m_logger) m_logger(message);
    });
}
```

> **重要约定**：所有回调都在**内部线程**上被调用，文档反复强调 *"The callback should not block"* —— 用户回调里做重活会拖垮信令/媒体线程。这是原生 WebRTC 封装的经典陷阱。

### 4.3 回调体系

```cpp
// 信令层
setOnSignalingConnectionOpened   // 连接建立
setOnSignalingConnectionClosed   // 连接关闭
setOnSignalingConnectionError    // 连接错误

// 房间层
setOnRoomClientsChanged          // 房间成员变化（vector<RoomClient>）

// 呼叫控制
setCallAcceptor                  // 返回 bool 决定是否接听（关键：可做权限控制）
setOnCallRejected                // 呼叫被拒
setOnClientConnected             // P2P 连接建立
setOnClientDisconnected          // P2P 断开
setOnClientConnectionFailed      // P2P 失败

setOnError / setLogger           // 错误与日志
```

`invokeIfCallable` 模板在所有事件回调前检查 `std::function` 是否有效：

```cpp
template<class T, class... Types>
void WebrtcClient::invokeIfCallable(const std::function<T>& f, Types... args) {
    callAsync(m_internalClientThread.get(), [=]() { if (f) f(args...); });
}
```

### 4.4 配置对象体系

六个配置类，统一用 `create()` 静态工厂而非公开构造器：

| 配置类 | 关键参数 |
|--------|----------|
| `SignalingServerConfiguration` | `url`, `clientName`, `room`, `password`, 自定义 `data` |
| `WebrtcConfiguration` | ICE 服务器列表 |
| `VideoStreamConfiguration` | `forcedCodecs`（VP8/VP9/H264）, `forceGStreamerHardwareAcceleration`, `useGStreamerSoftwareEncoderDecoder` |
| `AudioSourceConfiguration` | `soundCardTotalDelayMs` |
| `VideoSourceConfiguration` | 编码/分辨率相关开关 |
| `DataChannelConfiguration` | 通道参数 |

```cpp
enum class VideoStreamCodec { VP8, VP9, H264 };

// 强制使用 H264 + 硬件加速
auto cfg = VideoStreamConfiguration::create({VideoStreamCodec::H264}, true, false);
```

### 4.5 数据进出：Sources 与 Sinks

**推送侧（机器人 → 浏览器）**：

```cpp
auto videoSource = std::make_shared<VideoSource>(VideoSourceConfiguration::create(false, false));
videoSource->sendFrame(imageData, timestampUs);   // 推一帧
```

**接收侧（浏览器 → 机器人）**：

```cpp
client.setOnVideoFrameReceived([](const Client& c, const cv::Mat& img, uint64_t ts) { ... });
client.setOnEncodedVideoFrameReceived(...);   // 拿 H264/VP8 裸码流
client.setOnAudioFrameReceived(...);          // PCM
client.setOnMixedAudioFrameReceived(...);     // 混音后（多方通话）
```

### 4.6 PeerConnectionHandler：连接建立的实际执行者

继承自 libwebrtc 的三个观察者接口，是 SDP/ICE 交换的核心：

```cpp
class PeerConnectionHandler : public webrtc::PeerConnectionObserver,
                              public CreateSessionDescriptionObserverHelper,
                              public SetSessionDescriptionObserverHelper
{
    void makePeerCall();                                        // caller 侧：创建 offer
    void receivePeerCall(const std::string& sdp);               // callee 侧：收 offer → createAnswer
    void receivePeerCallAnswer(const std::string& sdp);         // caller 侧：收 answer
    void receiveIceCandidate(const std::string& sdpMid, int idx, const std::string& sdp);

    // Observer 回调
    void OnConnectionChange(PeerConnectionState newState) override;
    void OnIceCandidate(const webrtc::IceCandidateInterface*) override;
    void OnDataChannel(...) override;
    void OnTrack(...) / OnRemoveTrack(...) override;
};
```

`OnIceCandidate` 里捕获本地候选后，立刻通过信令发出去 —— **ICE 走信令通道，媒体走 P2P**。

---

## 五、GStreamer 硬件加速层（嵌入式关键）

在 Jetson 等平台上，用 CPU 编码 H264 会吃满核心，必须走硬件编码器。这一层把 GStreamer 的编解码器**注入到 libwebrtc 的编解码工厂链**里。

目录结构：

```
OpenteraWebrtcNativeGStreamer/
├── Encoders/     GStreamerVideoEncoder, H264/Vp8/Vp9GStreamerVideoEncoders
├── Decoders/     GStreamerVideoDecoder, H264/Vp8/Vp9GStreamerVideoDecoders
├── Factories/    WebRtcGStreamerVideoEncoderFactory / DecoderFactory
├── Pipeline/     GStreamerEncoderPipeline / GStreamerDecoderPipeline
└── Utils/
```

### 支持矩阵（README 原文）

| 平台 | VP8 | VP9 | H.264 |
|------|-----|-----|-------|
| Jetson TX2/Nano | `nvv4l2vp8enc` | `nvv4l2decoder` | `nvv4l2h264enc` |
| Jetson Xavier NX / AGX | ✗ | `nvv4l2decoder` | `nvv4l2h264enc` |
| **Jetson Orin / Orin Nano / Orin NX** | ✗ | `nvv4l2decoder` | `nvv4l2h264enc` |
| Raspberry Pi 4 | ✗ | ✗ | `v4l2h264enc` / `v4l2h264dec` |
| VA-API (Intel) | `vaapivp8enc` | `vaapivp9dec` | `vaapih264enc` |
| Apple Media | ✗ | ✗ | `vtenc_h264` / `vtdec` |

> **结论**：嵌入式平台上 **H.264 是唯一有完整硬编硬解支持的选择**（VP8 在 Jetson 上完全没有硬编）。VP9 编码仍在开发中（issue #117）。

---

## 六、Web 客户端（JavaScript）

与原生端**完全对称的事件协议**：

```
src/
├── index.js                     导出 {devices, iceServers, DataChannelClient,
│                                        StreamClient, StreamDataChannelClient}
├── WebrtcClient.js              基类（对应 C++ WebrtcClient）
├── StreamClient.js              音视频（继承 WebrtcClient）
├── DataChannelClient.js         数据通道
├── StreamDataChannelClient.js   音视频 + 数据通道
├── devices.js                   摄像头/麦克风枚举（getUserMedia）
├── iceServers.js                ICE 配置获取
└── Signaling/
    ├── SignalingClient.js          抽象基类（禁止直接实例化）
    └── WebSocketSignalingClient.js WebSocket 实现
```

事件映射与 C++ 端一字不差（`join-room-answer` / `room-clients` / `make-peer-call` / `peer-call-received` / `peer-call-answer-received` / `ice-candidate-received` / `close-all-peer-connections-request-received`），**这就是跨语言统一协议的威力**。

抽象类保护（JS 惯用法）：

```js
class SignalingClient {
  constructor(signalingServerConfiguration, logger) {
    if (this.constructor === SignalingClient) {
      throw new TypeError('Abstract class "SignalingClient" cannot be instantiated directly.');
    }
    ...
  }
}
```

---

## 七、Python 绑定与使用示例

pybind11 把 C++ 类包成 Python 模块 `opentera_webrtc.native_client`，**API 命名转 snake_case**：

```python
import opentera_webrtc.native_client as webrtc

# 1. 配置
signaling_server_configuration = webrtc.SignalingServerConfiguration.create_with_data(
    'ws://localhost:8080/signaling', 'Python', None, 'chat', 'abc')
    #  url,                             clientName, data, room,  password

ice_servers = webrtc.IceServer.fetch_from_server('http://localhost:8080/iceservers', 'abc')
webrtc_configuration = webrtc.WebrtcConfiguration.create(ice_servers)
video_stream_configuration = webrtc.VideoStreamConfiguration.create()

# 2. 数据源
video_source = webrtc.VideoSource(webrtc.VideoSourceConfiguration.create(False, False))
audio_source = webrtc.AudioSource(
    webrtc.AudioSourceConfiguration.create(0), 16, 48000, 1)  # delay_ms, bits, fs, channels

# 3. 客户端
client = webrtc.StreamClient(signaling_server_configuration, webrtc_configuration,
                             video_stream_configuration, video_source, audio_source)

# 4. 挂回调（属性赋值风格，非 setXxx）
client.on_signaling_connection_opened = on_signaling_connection_opened
client.on_room_clients_changed     = on_room_clients_changed
client.on_client_connected         = on_client_connected
client.on_video_frame_received     = on_video_frame_received   # (client, image, timestampUs)
client.on_audio_frame_received     = on_audio_frame_received

# 5. 连接并推送
client.connect()
video_source.send_frame(frame_with_noise, timestamp_us)   # numpy/OpenCV 图像直接推
audio_source.send_frame(audio_frame)                      # int16 PCM
```

**Python 侧的坑（注释里明确警告）**：
- 回调运行在**内部客户端线程**，不要阻塞
- `on_video_frame_received` 运行在 **WebRTC 处理线程**，与内部线程不同 —— 在 Python 里跨线程访问共享对象要加锁（GIL 只保护字节码，不保护逻辑一致性）

---

## 八、与 ROS 2 的集成

配套项目 [opentera-webrtc-ros](https://github.com/introlab/opentera-webrtc-ros)（ROS 2 Humble）：

- 把 ROS topic 上的 `sensor_msgs/Image` 直接推入 `VideoSource`
- 提供 C++ 和 Python 两种 ROS 节点封装
- 支持硬件加速与编解码器选择

这对机器人遥操作是完整闭环：**ROS 采图 → WebRTC 推流 → 浏览器实时显示 + 反向控制指令走数据通道**。

---

## 九、设计启示（学习要点）

1. **信令与媒体分离是 WebRTC 架构的第一原则**
   中心服务器只做"撮合"（房间、SDP/ICE 转发），媒体一旦协商好就 P2P 直连。服务器压力小、延迟低。

2. **跨语言协议统一降低心智负担**
   C++/Python/JS 三端用**完全相同的事件名和 JSON 结构**，`WebSocketSignalingClient.cpp` 与 `WebSocketSignalingClient.js` 的事件分支几乎逐行对应。定义一次协议，三端实现。

3. **线程边界必须显式化**
   所有对外 API 通过 `callSync`/`callAsync` 包裹，状态只在内部线程读写。回调文档统一标注 "should not block"。这是 C++ 封装异步库的标准做法。

4. **配置对象 + 静态工厂模式**
   6 个 `XxxConfiguration` 类都用 `create()` 而非公开构造器。好处：可读性（消除"5 个 bool 参数谁是谁"）、易扩展（加参数不破 ABI）、可与 pybind11 结合。

5. **抽象基类 + 虚工厂方法**
   `WebrtcClient` 用 `createPeerConnectionHandler()` 纯虚函数，让 `StreamClient`/`DataChannelClient` 各自提供 handler，基类负责全部信令与线程管理 —— 模板方法模式。

6. **硬件加速要抽象成"工厂注入"**
   GStreamer 编解码器通过 Factory 注入 libwebrtc，而不是改 libwebrtc 本体。平台差异（`nvv4l2h264enc` vs `v4l2h264enc`）被收敛在编解码器实现里。

7. **服务器防呼叫风暴**
   客户端只发 `call-ids`，由服务器计算不重复的呼叫对并下发 `make-peer-call`。把 O(N²) 的协调逻辑集中到一处。

8. **懒初始化 asyncio.Lock**
   `asyncio.Lock()` 绑定事件循环，必须在循环内创建 —— `_create_lock_if_none()` 是标准解法。

---

## 十、构建与依赖速查

```bash
# 依赖（3rdParty 目录下为 git submodule）
opentera-webrtc-native-client/3rdParty/
├── webrtc_native    # Google libwebrtc（预编译）
├── IXWebSocket      # WebSocket 客户端
├── opencv           # 图像处理
├── cpp-httplib      # HTTP（查 ICE 配置）
├── cpp-subprocess   # 子进程（GStreamer pipeline）
├── json             # nlohmann/json
├── googletest       # 测试
└── pybind11         # Python 绑定
```

信令服务器依赖：`aiohttp`、`aiohttp-index`（见 `signaling-server/requirements.txt`）

---

## 十一、对这个项目的评价

**优点**
- 三端统一协议，工程完成度高（含测试、CI、Doxygen 文档）
- 硬件加速支持矩阵覆盖主流嵌入式平台（Jetson 全家桶 + 树莓派 + VA-API + Apple）
- 回调 + 线程模型设计规范，是学习 C++ 异步封装的良好范本
- 与 ROS 2 生态有现成集成

**局限**
- **VP9 硬编码缺失**，嵌入式端实际只能用 H.264
- 信令服务器功能朴素（`README` 直接写着 "TODO documentation"），无持久化、无鉴权体系（仅全局口令）
- 最后提交 2025-04，活跃度已下降
- 构建链重（需预编译 libwebrtc），不适合轻量场景

**适用判断**：如果目标是在嵌入式平台上做**机器人 ↔ 浏览器的低延迟音视频遥操作**，且需要硬件编码，这个项目是目前开源方案里最完整的之一。若只需数据交换（不需要音视频），用现成的 WebSocket/MQTT 更简单。
