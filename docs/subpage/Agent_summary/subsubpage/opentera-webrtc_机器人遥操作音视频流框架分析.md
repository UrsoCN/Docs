---
created: 2026-08-20
modified: 2026-08-20
tags:
  - type/article
  - topic/webrtc
  - topic/robotics
  - lang/cpp
---
# OpenTera WebRTC 生态：音视频框架 + ROS 2 遥操作集成

> 本文覆盖 IntRoLab 的两个姊妹仓库：
>
> | 仓库 | 定位 | 版本/状态 |
> |------|------|-----------|
> | [introlab/opentera-webrtc](https://github.com/introlab/opentera-webrtc) | **基础层**：C++/Python/JS 客户端库 + 信令服务器 | 1.2.6（2025-04） |
> | [introlab/opentera-webrtc-ros](https://github.com/introlab/opentera-webrtc-ros) | **集成层**：ROS 2 (Humble) 节点封装，9 个 ROS 包 | 活跃开发中 |
>
> **机构**: IntRoLab, Université de Sherbrooke（加拿大舍布鲁克大学智能机器人实验室）
> **授权**: Apache 2.0（注意：默认构建的 libwebrtc 含非自由编解码器）
> **语言**: C++14（核心）+ Python 3（pybind11 / 节点脚本）+ JavaScript + CMake
> **依赖**: libwebrtc（Google 原生）、GStreamer（可选）、IXWebSocket、OpenCV、pybind11、nlohmann/json、protobuf、Qt5、RTAB-Map、audio_utils、ODAS

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

## 八、ROS 2 集成层（opentera-webrtc-ros）

这是把上面那套 WebRTC 能力**真正接到机器人上**的一层。相比基础库，它的工程量更大也更有参考价值——9 个 ROS 2 包，覆盖从推流、遥操作、导航到隐私保护的全链路。

### 8.1 包结构总览

| 包 | 语言 | 职责 |
|----|------|------|
| **opentera_webrtc_ros** | C++ | ⭐ 核心桥接：流节点、数据通道、JSON 指令分发 |
| **opentera_webrtc_ros_msgs** | msg | 24 个消息定义（Peer*/OpenTeraEvent/Waypoint/Label 等） |
| **opentera_client_ros** | Python | 连接 OpenTera 云平台，接收云端呼叫事件 |
| **opentera_protobuf_messages** | proto | 与 OpenTera 平台通信的 protobuf 定义 |
| **opentera_webrtc_robot_gui** | C++/Qt5 | 机器人端触屏 GUI（视频显示、麦克风/摄像头控制） |
| **opentera_webrtc_demos** | launch | Gazebo 仿真演示（含 TurtleBot3） |
| **map_image_generator** | C++ | 把导航地图 + 激光 + 机器人位姿绘制成 2D 图像推流 |
| **face_cropping** | C++/LibTorch | ⭐ 人脸检测裁剪，隐私保护 + 带宽优化 |
| **turtlebot3_beam_description** | URDF | 演示用机器人模型 |

### 8.2 核心设计：RosWebRTCBridge 模板基类

整个集成层最精妙的地方是一个 **543 行的模板基类**，把"ROS 节点"与"WebRTC 客户端"嫁接起来：

```cpp
template<typename T>
class RosWebRTCBridge : public rclcpp::Node
{
    static_assert(std::is_base_of<WebrtcClient, T>::value,
                  "T must inherit from opentera::WebrtcClient");
    ...
    std::unique_ptr<T> m_signalingClient;   // 由子类决定是 StreamClient 还是 DataChannelClient
};
```

对比基础库的 `WebrtcClient` 用**虚函数工厂**（运行期多态），这里用**模板参数**（编译期多态）——因为节点类型在编译期就确定了，省掉虚函数开销，也让编译器能内联全部回调。

**双向事件桥**：

```
ROS 侧                          WebrtcClient 侧
─────────────────────────────────────────────────
订阅 "events"（云端事件）  →  onDeviceEvents / onJoinSessionEvents /
                              onParticipantEvents / onStopSessionEvents ...
                              → connect() / disconnect() 信令连接

发布 "webrtc_peer_status"  ←  onSignalingConnectionOpened / onClientConnected /
                              onClientDisconnected / onClientConnectionFailed
```

**云端事件全集**（`opentera_webrtc_ros_msgs`）：

`OpenTeraEvent`(总容器) → `DatabaseEvent` / `DeviceEvent` / `JoinSessionEvent` / `JoinSessionReplyEvent` / `LeaveSessionEvent` / `LogEvent` / `ParticipantEvent` / `StopSessionEvent` / `UserEvent`

这是**会话状态机**——不是简单的 p2p 呼叫，而是完整的"用户/设备/参与者"三方会话模型。

### 8.3 RosStreamBridge：音视频双向桥

**订阅**：`ros_image` (`sensor_msgs/Image`)、`audio_in` (`audio_utils_msgs/AudioFrame`)
**发布**：`webrtc_image` (`PeerImage`)、`webrtc_audio` (`PeerAudio`)、`audio_mixed` (`AudioFrame`)

参数体系（ROS param XML）：

```xml
<param name="is_stand_alone" value="true"/>          <!-- 独立模式 vs 云平台模式 -->
<param name="stream">
  <param name="can_send_audio_stream"    value="true"/>   <!-- 四路能力开关 -->
  <param name="can_receive_audio_stream" value="true"/>
  <param name="can_send_video_stream"    value="true"/>
  <param name="can_receive_video_stream" value="true"/>
  <param name="is_screen_cast"           value="false"/>  <!-- 屏幕共享 vs 摄像头 -->
  <param name="needs_denoising"          value="false"/>
</param>
<param name="signaling">
  <param name="server_url"    value="http://localhost:8080"/>
  <param name="client_name"   value="streamer"/>
  <param name="room_name"     value="chat"/>
  <param name="room_password" value="abc"/>
</param>
```

> 设计亮点：`can_send/receive_*_stream` 四个开关决定 `VideoSource`/`AudioSource` 是否被创建 —— 收流端不需要构造发送源，省内存。

**音频处理参数**（直接透传给 WebRTC 音频模块）：

```cpp
m_nodeParameters.loadAudioStreamParams(
    m_canSendAudioStream, m_canReceiveAudioStream,
    soundCardTotalDelayMs,      // 声卡总延迟（AEC 参考信号对齐关键！）
    echoCancellation,           // 回声消除
    autoGainControl,            // 自动增益
    noiseSuppression,           // 降噪
    highPassFilter,             // 高通滤波
    stereoSwapping,
    transientSuppression);      // 瞬态抑制
```

> **`soundCardTotalDelayMs` 是 AEC 能工作的前提**——它告诉 WebRTC 扬声器信号到麦克风的延迟，用来对齐参考信号。这个值和硬件的 buffer 深度强相关。

### 8.4 RosVideoSource：ROS 图像的预处理

```cpp
void RosVideoSource::sendFrame(const sensor_msgs::msg::Image::ConstSharedPtr& msg)
{
    cv::Mat bgr;
    if (msg->encoding.find("F") != std::string::npos)   // 判断是否是"浮点"图像
    {
        // 浮点图（如深度图/归一化图）需要先按最大值缩放到 [0,255]
        cv::Mat_<float> float_image = cv_bridge::toCvShare(msg, msg->encoding)->image;
        double max_val;
        cv::minMaxIdx(float_image, 0, &max_val);
        if (max_val > 0) float_image *= (255 / max_val);
        cv::Mat orig;
        float_image.convertTo(orig, CV_8U);
        cv::cvtColor(orig, bgr, CV_GRAY2BGR);
    }
    else
    {
        bgr = cv_bridge::toCvShare(msg, "bgr8")->image;
    }
    int64_t camera_time_us = to_microseconds(msg->header.stamp);
    VideoSource::sendFrame(bgr, camera_time_us);   // 时间戳用 ROS 的！不是本地时间
}
```

> 两个关键点：① 支持 float 图像自动归一化（深度图可直接推流）；② **时间戳透传 ROS header stamp**，保证多路流的时间一致性。

### 8.5 遥操作协议：JSON over DataChannel

`RosDataChannelBridge` 负责 `std_msgs/String` ↔ WebRTC 数据通道搬运，`RosJsonDataHandler` 把 JSON 分发到具体 ROS 话题/服务。

**完整的指令集**（`RosJsonDataHandler.cpp` 实录）：

| `type` | 字段 | 映射目标 |
|--------|------|----------|
| `stop` / `start` | `state` | `std_msgs/Bool` → 导航启停 |
| `velCmd` | `x`, `yaw` | `geometry_msgs/Twist` → **`cmd_vel` 遥控** |
| `waypointArray` | `array[{coordinate{x,y,yaw}}]` | `WaypointArray` → 多点导航 |
| `action` | `action: dock` | 服务 `do_docking` |
| `action` | `action: localizationMode` / `mappingMode` | 服务 `/rtabmap/rtabmap/set_mode_*` |
| `action` | `action: setMovementMode` / `doMovement` | 服务 `set_movement_mode` / `do_movement` |
| `micVolume` / `volume` | 值 | `std_msgs/Float32` → 音量控制 |
| `enableCamera` | 值 | `std_msgs/Bool` |
| `changeMapView` | — | 服务 `change_map_view` |
| `goToLabel` / `removeLabel` / `addLabel` / `editLabel` | 标签信息 | 语义导航点管理 |

速度遥控的核心逻辑：

```cpp
else if (serializedData["type"] == "velCmd")
{
    geometry_msgs::msg::Twist twist;
    // Multiply by 0.15 in order to control the speed of the movement
    twist.linear.x  = static_cast<double>(serializedData["x"])   * m_linear_multiplier;   // 默认 0.15
    twist.angular.z = static_cast<double>(serializedData["yaw"]) * m_angular_multiplier;  // 默认 0.15
    m_cmdVelPublisher->publish(twist);
}
```

> **安全设计**：前端（浏览器摇杆）发来的是 **-1.0 ~ 1.0 的归一化值**，节点侧乘以 `linear_multiplier`(0.15) 转换成实际速度。这样"最大速度"由机器人侧参数决定，**前端无法越权发送超速指令**。yaw 用 `M_PI/180` 做角度→弧度转换。

### 8.6 map_image_generator：把导航状态画成图推流

这是本项目的另一个巧思：**不做 3D 可视化，而是把地图渲染成一张 2D 图像直接推流**。

```
订阅: /map (OccupancyGrid)     → OccupancyGridImageDrawer  栅格地图
      /scan (LaserScan)        → LaserScanImageDrawer      激光点
      /odom, /tf               → RobotImageDrawer          机器人位姿
      /global_plan             → GlobalPathImageDrawer     全局路径
      /goal_pose               → GoalImageDrawer           目标点
      /sound_source_localization → SoundSourceImageDrawer  声源定位(ODAS)
      标签                     → LabelImageDrawer          语义标签

发布: 一张合成好的 sensor_msgs/Image → 喂给 RosVideoSource → WebRTC 推流
服务: 图像坐标 ↔ 地图坐标转换（供 goal_manager / labels_manager 使用）
```

> 为什么这么做？**因为浏览器端不需要装 RViz**——一张图就能让操作员看到地图、机器人位置、路径规划和声源方向。代价是失去交互性和 3D 视角，换来极低的实现与带宽成本。README 说明"暂时以 2D 图像流简化处理"。

### 8.7 导航目标管理：goal_manager 与 labels_manager

两个 Python 节点，共用 `libnavigation` 库：

- **`goal_manager`**：接收前端发来的多个图像坐标航点 → 通过 `map_image_generator` 的服务转成地图坐标 → **逐个**发给 nav2，到达后发布 `waypoint_reached`（JSON）通知前端
- **`labels_manager`**：管理**语义标签**（名字 + 位姿 + 描述），存 YAML（`libyamldatabase`）。支持按名字导航、增删改。⚠️ 标签绑定地图坐标系，**地图变了数据库必须清理**

> 这两个节点实现了"**点击地图 → 机器人自动导航**"的完整体验：操作员在浏览器地图上点几个点，机器人就依次跑过去。

### 8.8 face_cropping：隐私保护 + 带宽优化（很有想法）

远程康复场景涉及真实患者/家属，直接推完整画面有隐私风险。这个节点**检测画面中最大的人脸并裁剪输出**。

**模型选型与性能实测**（AMD Ryzen 7 3700X @ 30Hz，WIDER FACE 子集）：

| 模型 | 需 LibTorch | CPU (%/核) | 内存 (MB) | AP@0.25 | AP@0.50 | AP@0.75 |
|------|------------|-----------|----------|---------|---------|---------|
| haarcascade | ✗ | 220.4 | 299.7 | 0.5973 | 0.5649 | 0.0314 |
| lbpcascade | ✗ | 154.7 | 299.1 | 0.4141 | 0.4014 | 0.0705 |
| **small_yunet_0.5_320** | ✓ | **20.2** | 282.2 | **0.8622** | **0.8034** | 0.4840 |
| small_yunet_0.5_640 | ✓ | 50.1 | 315.8 | 0.8780 | 0.8466 | **0.6018** |
| small_yunet_0.25_160 | ✓ | 11.4 | 273.9 | 0.6896 | 0.4940 | 0.1298 |

> **结论**：传统 Haar/LBP 级联（`haarcascade` 用 220% CPU！）在现代分辨率下已经不可用；自研的 `small_yunet_*` 用**知识蒸馏**压缩骨干网（ImageNet 预训练 + 蒸馏），`0.5_320` 型号在 **20% CPU** 下拿到 0.80 AP@0.5 —— 比 haarcascade 准得多还快 10 倍。
>
> 模型训练代码在 `dnn_training/`（含 SimOTA 正负样本分配，取自 MMDetection）。Jetson 上需自行编译 LibTorch + torchvision，或用 `FACE_CROPPER_USE_CUDA=ON` 走 GPU。

参数：`min_face_width/height`（过滤小脸）、`output_width/height`、`adjust_brightness`（提亮）、`use_gpu_if_available`。可由机器人 GUI 的按钮实时开关。

### 8.9 opentera_client_ros：云端接入

不再是"局域网里连信令服务器"，而是作为**设备**注册到 OpenTera 云平台：

```json
// config/client_config.json
{
  "client_token": "JWT token generated from the OpenTera server",
  "url": "https://server:port"
}
```

响应 7 类平台事件：`DeviceEvent`（设备上下线）、`JoinSessionEvent` / `JoinSessionReplyEvent`（会话加入）、`ParticipantEvent` / `UserEvent`（参与者/用户在线状态）、`StopSessionEvent`（会话终止）、`LeaveSessionEvent`（离开）。

会话由 [opentera-teleop-service](https://github.com/introlab/opentera-teleop-service) 的 webportal 发起 —— 即**治疗师在网页上发起呼叫，机器人被叫响**。

### 8.10 机器人端 GUI（Qt5）

`opentera_webrtc_robot_gui`：机器人本体的触摸屏界面。展示远端视频流、本地摄像头画中画（可拖动/缩放、透明度可调）、显示呼叫信息、控制麦克风/摄像头/人脸裁剪开关。

通过 `deviceProperties` JSON 适配不同屏幕（分辨率、对角尺寸、画中画位置），可用 ROS 参数 `device_properties_path` 覆盖。README 坦承"GUI 尚未完成，但已能显示基本视频流"。

### 8.11 快速上手（Gazebo 仿真）

```bash
# 依赖（Ubuntu 22.04 + ROS 2 Humble）
sudo apt install ros-humble-rtabmap-ros ros-humble-turtlebot3-gazebo \
                 ros-humble-turtlebot3-navigation2 libprotobuf-dev \
                 python3-protobuf portaudio19-dev nodejs npm \
                 libqt5charts5-dev libgstreamer1.0-dev ...

# 拉取（含依赖包）
cd ~/teleop_ws/src
git clone https://github.com/Kapernikov/cv_camera.git
git clone https://github.com/introlab/audio_utils.git --recurse-submodules
git clone https://github.com/introlab/odas_ros.git --recurse-submodules
git clone https://github.com/introlab/opentera-webrtc-ros.git --recurse-submodules

# 构建
cd ~/teleop_ws
colcon build --symlink-install --cmake-args -DPYTHON_EXECUTABLE=/usr/bin/python3

# 跑仿真演示
source install/setup.bash
ros2 launch opentera_webrtc_demos demo.launch.xml is_stand_alone:=true
# 浏览器打开 http://localhost:8080/index.html#/user?name=dev&pwd=abc&robot=BEAM
```

> 需要 CMake ≥ 3.22（Ubuntu 22.04 自带 3.22 需按官方指引升级）。

---

## 八·补、两仓库的调用栈关系

```
┌─────────────────────────────────────────────────────────────┐
│ opentera-webrtc-ros （ROS 2 集成层）                          │
│                                                              │
│  RosWebRTCBridge<T>  ← 模板基类，ROS 节点 × WebrtcClient      │
│    ├── RosStreamBridge        → StreamClient                 │
│    └── RosDataChannelBridge   → DataChannelClient            │
│                                                              │
│  RosVideoSource → VideoSource      RosAudioSource → AudioSource│
│  RosJsonDataHandler（JSON → cmd_vel / nav2 / rtabmap）         │
│  goal_manager / labels_manager（导航与语义点）                 │
│  map_image_generator（地图渲染成图推流）                        │
│  face_cropping（隐私裁剪，LibTorch）                           │
│  opentera_client_ros（OpenTera 云端设备接入）                  │
└──────────────────────────┬──────────────────────────────────┘
                           │ 直接依赖（源码级，非 ROS 包）
                           ▼
┌─────────────────────────────────────────────────────────────┐
│ opentera-webrtc （基础层）                                     │
│  StreamClient / DataChannelClient / WebrtcClient            │
│  WebSocketSignalingClient（信令协议 v2）                      │
│  GStreamer 编解码工厂（硬件加速）                               │
└──────────────────────────┬──────────────────────────────────┘
                           ▼
        libwebrtc (Google) + 信令服务器 (Python/aiohttp)
```

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

### 来自 ROS 集成层的补充启示

9. **编译期多态 vs 运行期多态的选择**
   基础库 `WebrtcClient` 用虚工厂（运行期多态）让用户自选 client 类型；集成层 `RosWebRTCBridge<T>` 改用模板参数（编译期多态），因为节点类型编译期已知。**同一套代码，不同的多态策略**——这是很好的设计取舍案例。

10. **安全边界前移到机器人侧**
    前端发归一化值（-1.0~1.0），机器人侧乘 `linear_multiplier` 才变成实际速度。**"最大速度"永远由被控端决定**，前端即使被篡改也无法让机器人超速。这个模式可推广到任何远程控制协议。

11. **用"图像流"替代 3D 可视化客户端**
    `map_image_generator` 把地图/激光/位姿/路径画成一张图推流，浏览器端不需要 RViz 或任何地图引擎。**牺牲交互性换实现与带宽成本**——在"看得到就行"的遥操作场景是明智取舍。

12. **推流链路上做隐私过滤**
    `face_cropping` 在 ROS→WebRTC 之间插入一道人脸检测裁剪。相比在客户端做模糊（视频已出网），**在源头裁剪才是真隐私保护**。同时顺带降低带宽。

13. **时间戳必须透传源头**
    `RosVideoSource` 用 `msg->header.stamp` 而非本地时钟。多路流（摄像头/地图）要能对齐，时间源必须统一。

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

## 十一、总结与评价

### 分层价值

| 层 | 你能学到什么 |
|----|-------------|
| **opentera-webrtc**（基础层） | 如何封装 libwebrtc：线程模型、回调设计、配置对象、跨语言统一协议、硬件加速工厂注入 |
| **opentera-webrtc-ros**（集成层） | 如何把 WebRTC 落到 ROS 2 机器人上：模板化节点桥接、JSON 遥操作协议、导航可视化、隐私过滤、云端会话模型 |

### 优点

- **两个仓库职责清晰**：基础层与 ROS 层解耦，基础层可复用于非 ROS 场景（如纯 Python 服务）
- 三端统一协议，工程完成度高（含测试、CI、Doxygen 文档）
- 硬件加速支持矩阵覆盖主流嵌入式平台（Jetson 全家桶 + 树莓派 + VA-API + Apple）
- 回调 + 线程模型设计规范，是学习 C++ 异步封装的良好范本
- **ROS 层有真实场景沉淀**：远程康复（患者隐私、治疗师遥操作、语义导航点）——不是玩具 demo
- `face_cropping` 的模型对比数据（CPU/AP 权衡）是可复用的工程选型参考

### 局限

- **VP9 硬编码缺失**，嵌入式端实际只能用 H.264
- 基础库最后提交 **2025-04**，活跃度已下降（ROS 仓库仍在开发）
- 信令服务器功能朴素（`README` 直接写着 "TODO documentation"），**无持久化、无细粒度鉴权**（仅全局口令）
- **云端会话能力依赖 OpenTera 平台**：`opentera_client_ros` 需要 OpenTera 服务器 + teleop-service，不接入平台时只能用 stand-alone 模式
- 构建链重（预编译 libwebrtc + protobuf + RTAB-Map + Qt5 + GStreamer 一整套），不适合轻量场景
- 部分组件自认未完成：robot_gui "unfinished"、VP9 编码开发中

### 适用判断

| 场景 | 建议 |
|------|------|
| 嵌入式平台上做**机器人 ↔ 浏览器低延迟音视频遥操作 + 硬件编码** | ✅ 目前开源方案里最完整的之一 |
| 需要**语音交互 + 声源定位**（ODAS 集成） | ✅ 有现成集成，可参考 |
| ROS 2 机器人要做**云端远程诊疗/运维** | ✅ opentera_client_ros + teleop-service 是完整参考实现 |
| 只需要传数据（无音视频） | ❌ 用 WebSocket/MQTT 更简单 |
| 需要 VP8/VP9 硬件编码 | ❌ 只有 H.264 可用 |
| 轻量/快速集成 | ❌ 构建链太重 |

### 与本项目的相关点（机器狗 / 语音交互）

- **`soundCardTotalDelayMs`**：音频 AEC 参数——与语音交互系统的回声消除直接相关，值需按硬件 buffer 实测调整
- **`AudioSource`/`AudioSink` 接口设计**：可参考其"推帧/收帧 + 时间戳"的音频抽象
- **ODAS 声源定位集成**（`demo_odas.launch.xml`）：与多麦阵列（如 XVF3800）方案可对照
- **`can_send/receive_*` 四开关**：能力开关式参数设计，值得在自己项目里借鉴
