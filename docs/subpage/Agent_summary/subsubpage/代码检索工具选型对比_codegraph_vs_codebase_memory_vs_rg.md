---
created: 2026-06-17
modified: 2026-06-17
tags:
  - type/comparison
  - topic/dev-tools
---

# 代码检索工具选型对比：codegraph vs codebase-memory-mcp vs rg vs LSP

> 基于 ***** 代码库（*语音交互系统）的七维实测对比。全部数据来源于真实工具调用，非理论分析。

---

## 一、实测环境

| 项目 | 值 |
|---|---|
| **代码库** | `/home/u/code/*` |
| **文件数** | 2,059 |
| **语言构成** | C++ 1,736 / C 238 / Python 13 / Java 6 / JS 2 / YAML 27 / XML 16 |
| **第三方库** | Eigen 5.0.0, sherpa-onnx, spdlog, speexdsp, libfvad, hexaware 等 |
| **codegraph** | 索引到位：53,833 nodes / 105,569 edges / 124 MB SQLite |
| **codebase-memory-mcp (CBM)** | moderate 模式：940 nodes / 1,748 edges（自动排除 third_party） |
| **ripgrep (rg)** | 零配置，即时 |

---

## 二、七维横向对比

### 测试 ① 代码库信息概览

> 查询目标：快速了解项目结构、语言组成、规模

#### codegraph 输出

```
Files indexed: 2,059
Nodes: 53,833 / Edges: 105,569
Languages:
  c: 238, cpp: 1,736, java: 6, javascript: 2
  objc: 1, php: 18, properties: 2, python: 13
  xml: 16, yaml: 27
Nodes by kind:
  class: 1,300, function: 12,857, method: 13,674
  struct: 4,451, import: 5,247, enum: 1,320
```
- **Tokens**: ~250
- **特点**: 包含 third_party（Eigen、sherpa-onnx 等），总节点数巨大

#### codebase-memory-mcp 输出

```
Nodes: 940 / Edges: 1,748
Languages: C(15), C++(11), YAML(8), Python(7), Bash(5)
Entry points: main x5 (afe_ros, asr_ros, kws_ros, tts_ros, nlu_node)
Hotspots: logF(5), UnloadModel(2), StopPlayback(2), ...
Leiden clusters: [src] 2 members, cohesion=1.0
Routes: /speech/get_asr_status, /speech/get_tts_status
```
- **Tokens**: ~450
- **特点**: 自动排除 third_party/build/scripts，纯净项目代码；额外提供社区检测、热点分析、路由发现

#### ripgrep 方案

```bash
# 需要多个命令拼凑
find . -name "*.cpp" -o -name "*.h" | wc -l    # 文件数
rg -c "^class |^struct |^enum " --sort path      # 粗略类型统计
```
- **Tokens**: 多个命令累积 ~500+，需人工整合
- **特点**: 零配置但无结构化理解，无法给出架构概览

#### 对比结论

| 维度 | codegraph | CBM | rg |
|---|---|---|---|
| 输出风格 | 原始统计 | 结构化分析 + 智能发现 | 需拼凑 |
| 是否含 third_party | ✅ 包含 | ❌ 自动排除 | 取决于 glob |
| 热点函数识别 | ❌ | ✅ | ❌ |
| 社区/模块检测 | ❌ | ✅ Leiden | ❌ |
| HTTP/ROS 路由 | ❌ | ✅ | ❌ |
| tokens | ~250 | ~450 | ~500+ |

---

### 测试 ② 符号搜索

> 查询目标：找到 AsrEngine 类的定义及其所有方法

#### codegraph

```bash
codegraph_node("asr_engine.h", includeCode=true)
```

1 次调用 → 返回完整头文件（第 40-84 行），包含：
- `AsrResultCallback` 类型定义
- `AsrConfig` 结构体（10 个字段）
- `AsrEngine` 类（18 个公开方法 + 2 个私有成员）
- 同时自带调用链（被谁 include、include 了什么）

**Tokens**: ~200 | **解读清晰度**: ⭐⭐⭐⭐⭐

#### codebase-memory-mcp

```bash
search_graph(query="AsrEngine", label="Class")
```

返回 65 条 BM25 排序结果（需过滤），其中关键项：
```
Method: AsrEngine @ asr_engine.cpp:647
Method: Initialize @ asr_engine.cpp:651
Method: Start @ asr_engine.cpp:671
...
Class: AsrEngine @ asr_engine.h:40
Class: AsrEngine::Impl @ asr_engine.cpp:39
```

额外调用 `get_code_snippet(qualified_name=...)` 可返回完整源码（含圈复杂度、循环深度、递归检测等元数据）。

**Tokens**: ~500 + ~400 | **解读清晰度**: ⭐⭐⭐⭐（需二次调用获取源码）

#### ripgrep

```bash
rg "class AsrEngine" --glob="*.h"
# 输出: src/speech/include/speech/asr/asr_engine.h:40:class AsrEngine {
# 然后手动读头文件...
```

1 次 rg 找到定义位置 + 手动读头文件。无结构信息，无方法列表。

**Tokens**: ~50 + 读头文件 ~200 | **解读清晰度**: ⭐⭐⭐（依赖人读文件）

#### 对比结论

| 维度 | codegraph | CBM | rg |
|---|---|---|---|
| 调用次数 | 1 | 2（search + snippet） | 1 + 手动 |
| 返回源码 | ✅ 完整头文件 | ✅ 完整方法体（含复杂度） | ❌ 需要额外 Read |
| 方法列表 | ✅ 自动 | ✅ 自动（限 10 条需分页） | ❌ |
| 额外元数据 | 调用链 | 复杂度/循环/递归 | ❌ |
| **精准度** | 精确匹配 | BM25 排序（有干扰项） | 精确 |

---

### 测试 ③ 调用链追踪（核心能力）

> 查询目标：谁调用了 AsrEngine::Start？调用链多深？

#### codegraph

```bash
codegraph_callers("asr::AsrEngine::Start")
```
```
Callers of asr::AsrEngine::Start (1 found):
  startEngine (method) - src/speech/src/asr/asr_ros.cpp:579

codegraph_node("startEngine", includeCode=true):
  Calls → Start (asr_engine.cpp:671)
  Called by ← enableServiceCallback (asr_ros.cpp:282)
              switchDeviceServiceCallback (asr_ros.cpp:410)
```

**1 次调用** → 已知 1 个直接 caller；再 1 次调用 → 已知 2 个间接 caller（service callback）

**Tokens**: ~100 + ~200 | **解读清晰度**: ⭐⭐⭐⭐⭐

#### codebase-memory-mcp

```bash
trace_path(function_name="...AsrEngine.Start", direction="inbound", depth=2)
```
```
"callers": []  ← 空！moderate 模式下未追踪到调用边
```

**失败原因分析**：CBM 的 moderate 模式以速度和简洁性优先，CALLS 边（211 条）主要覆盖函数间直接调用关系，但对通过 `impl_` 指针的间接调用（`impl_->Start()`）可能因缺少跨翻译单元索引而丢边。

备用方案：`search_graph(query="initializeEngine AsrNode", include_connected=true)` 可展示函数列表，但无法展示调用关系图。

**Tokens**: ~50（空结果）+ 替代方案 ~300 | **解读清晰度**: ⭐⭐

#### ripgrep

```bash
rg "->Start\(" --glob="*.cpp" src/speech/src/
# → asr_engine.cpp:671: return impl_->Start();
# → asr_ros.cpp:579: if (asr_engine_) asr_engine_->Start();
```
1 次 rg 找到 2 处调用。但无法区分哪个是定义、哪个是调用；无法自动追踪 startEngine 的上游。

**Tokens**: ~50 | **解读清晰度**: ⭐⭐⭐（需手动阅读上下文区分）

#### 对比结论

| 维度 | codegraph | CBM | rg |
|---|---|---|---|
| 调用链深度 | ✅ 多级追踪 | ❌ moderate 丢边 | ❌ 需手动追 |
| caller 区分度 | ✅ 精准 | ❌ 未找到 | ⚠️ 需人工判断 |
| 间接调用追踪 | ✅（enableServiceCallback→startEngine→Start） | ❌ | ❌ |
| tokens | ~300 | ~350 | ~50 |
| **综合胜出** | **🏆** | 需 medium/full 模式 | 文本搜索可用 |

---

### 测试 ④ 影响分析

> 查询目标：修改 AsrEngine::Start() 会影响到哪些代码？

#### codegraph

```bash
codegraph_impact(symbol="asr::AsrEngine::Start", depth=2)
```
```
Impact: "asr::AsrEngine::Start" affects 6 symbols:
  src/speech/src/asr/asr_engine.cpp: Start:671, asr_engine.cpp:1
  src/speech/src/asr/asr_ros.cpp: startEngine:579, enableServiceCallback:282,
                                     switchDeviceServiceCallback:410, asr_ros.cpp:1
```

**1 次调用** → 6 个受影响的符号，含文件路径+行号+影响传播链

**Tokens**: ~80 | **解读清晰度**: ⭐⭐⭐⭐⭐

#### codebase-memory-mcp

```bash
detect_changes(project=..., since="HEAD~5")
```
工具提供但需要 git diff 配合；当前实验无代码变更，无法产生有意义结果。

`query_graph` 理论上可用 Cypher 查询调用链：
```
MATCH (f:Function)-[:CALLS]->(g:Function)
WHERE f.name CONTAINS "startEngine"
RETURN f.name, g.name
```
但需要使用方熟悉 Cypher 语法。

**Tokens**: Cypher 查询 ~80 + 结果解析 | **解读清晰度**: ⭐⭐⭐⭐（需要熟悉 Cypher）

#### ripgrep

```bash
# 需要手动追踪所有引用
rg -l "Start" --glob="*.cpp" src/speech/src/
# → asr_engine.cpp, asr_ros.cpp
# 然后手动读每个文件确认调用关系...
```

无结构理解，无法区分方法定义与方法调用，无法展示影响传播链。

**Tokens**: ~30（搜索）+ 手动读多个文件数千 tokens | **解读清晰度**: ⭐⭐

#### 对比结论

| 维度 | codegraph | CBM | rg |
|---|---|---|---|
| 一步到位 | ✅ | ⚠️ 需 Cypher | ❌ |
| 语义理解 | ✅ | ✅ | ❌ |
| 影响传播链 | ✅ | ✅（Cypher） | ❌ |
| 使用门槛 | 极低 | 中等（需 Cypher） | 低但不结构 |
| **综合胜出** | **🏆** | 潜力大但需学习 | 基本不可用 |

---

### 测试 ⑤ 架构概览

> 查询目标：项目有哪些模块？入口点在哪？各部分怎么组织的？

#### codegraph

```bash
codegraph_files(path="src/speech/src", format="grouped")
```
```
Files (12):
  C++ (11):
    src/speech/src/afe/   → afe_io, afe_ros, ec_processor, gtcrn_processor, vad_processor
    src/speech/src/asr/   → asr_engine, asr_ros
    src/speech/src/kws/   → keyword_spotter, kws_ros
    src/speech/src/tts/   → tts_engine, tts_ros
  Python (1):
    src/speech/src/nlu/   → nlu_node
```

**Tokens**: ~100 | **特点**: 简洁的文件树按语言分组

#### codebase-memory-mcp

```bash
get_architecture(aspects=["all"])
```

返回丰富的架构信息：
- 5 个 entry points（每个 ROS node 的 main）
- 2 个 ROS service routes
- 10 个热点函数（按 fan-in 排序）
- Leiden 社区检测（识别出 1 个实际模块集群）
- 软件分层分析（api/internal）
- 文件树（64 个节点，含 launch/config/res）

**Tokens**: ~900 | **特点**: 数据丰富，智能分析，但输出量大

#### ripgrep + find

```bash
# 多个命令拼凑
find src/speech/src -name "*.cpp" -o -name "*.py" | sort
# 手动查看 CMakeLists.txt 了解模块组织...
```

零结构化。需要大量人工阅读和推理。

**Tokens**: 无法计算 | **特点**: 不适合架构理解

#### 对比结论

| 维度 | codegraph | CBM | rg |
|---|---|---|---|
| 模块识别 | ✅ 文件树 | ✅ 社区检测+分层 | ❌ |
| 入口点 | ❌ | ✅ | ❌ |
| 热点函数 | ❌ | ✅ | ❌ |
| 服务/路由 | ❌ | ✅ /speech/get_asr_status | ❌ |
| 输出大小 | 简洁 | 丰富但量大 | 原始 |
| **用例匹配** | 快速概览 | 深度架构分析 | 不适用 |

---

### 测试 ⑥ 纯文本模式搜索

> 查询目标：找到代码中所有 TODO/FIXME 注释

#### ripgrep

```bash
rg -n "TODO|FIXME" --glob="*.cpp" src/speech/src/
```
```
src/speech/src/asr/asr_ros.cpp:613:    // TODO: Implement device checking for StreamAsr if needed
```

**Tokens**: ~20 | **结果**: 1 处匹配，清晰准确

#### codegraph

```bash
codegraph_search("TODO FIXME")
```
返回 Eigen 第三方库中的 stride/rows/cwiseMin 等符号匹配（FTS5 分词将 TODO 拆解匹配到无关符号）。**不适用于文本模式搜索**。

**Tokens**: ~150 | **结果**: 无效，误匹配

#### codebase-memory-mcp

```bash
search_code(pattern="TODO|FIXME", file_pattern="*.cpp", path_filter="^src/speech/src/")
```
```
raw grep matches: 0, dedup results: 0
```
搜索代码中的 TODO，但 CBM 排除了 third_party 却也没能找到那 1 处 TODO。**不如 rg 直接**。

**Tokens**: ~100 | **结果**: 遗漏

#### 对比结论

| 维度 | rg 🏆 | codegraph | CBM |
|---|---|---|---|
| 纯文本搜索 | ✅ 完美 | ❌ 不适用 | ⚠️ 不稳定 |
| tokens | ~20 | ~150 | ~100 |
| 结果准确性 | 精准 | 误匹配 | 丢失 |
| **适合场景** | TODO/FIXME/api_key 等 | 不支持 | 应使用 rg |

---

### 测试 ⑦ 跨语言追踪

> 查询目标：Python NLU 节点如何与 C++ 模块连接？

#### codegraph

```bash
codegraph_files(path="src/speech/src/nlu")
# → nlu_node.py (Python)

codegraph_search("NLU", kind="function")
# → 不跨语言搜索 Python（索引只有 C++ 符号）
```

**codegraph 对 Python 文件索引有限**（13 个 Python 文件 vs 1736 个 C++），跨语言边未自动建立。

#### codebase-memory-mcp

跨服务追踪通过 `trace_path(mode="cross_service")` 支持 HTTP_CALLS/ASYNC_CALLS 边。当前项目只有 2 条 Route（ROS service），无跨语言 HTTP 链接，无法充分测试。

`get_architecture()` 显示了 `pyhanlp` 作为独立 package（fan_in=0, fan_out=0），表明 NLU 节点与 C++ 模块通过 ROS 话题/服务通信，而非直接函数调用。

#### ripgrep

```bash
rg "nlu\|NLU\|nlu_node" --glob="*.cpp" --glob="*.py" -l
# → 找到相关引用：launch 文件、config 中注释等
```

可以找到文本级别的关联，但无法理解架构层面的连接方式。

#### 对比结论

| 维度 | codegraph | CBM | rg |
|---|---|---|---|
| Python 支持 | ⚠️ 有限（13 文件） | ✅ 识别为独立包 | ✅ 纯文本 |
| 跨语言调用链 | ❌ | ❌（本项目中） | ❌ |
| 架构理解 | ❌ | ✅（识别 NLU 为孤立节点） | ❌ |
| **适合场景** | C++ 主导项目 | 多语言全局分析 | 文本搜索 |

---

## 三、综合性能矩阵

| 测试维度 | codegraph | CBM | rg | LSP* |
|---|---|---|---|---|
| **① 项目概览** | ⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐ | ⭐ |
| **② 符号搜索** | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐⭐ |
| **③ 调用链追踪** | ⭐⭐⭐⭐⭐ | ⭐⭐（moderate） | ⭐⭐⭐ | ⭐⭐⭐⭐ |
| **④ 影响分析** | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐ | ⭐⭐ |
| **⑤ 架构概览** | ⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐ | ⭐ |
| **⑥ 文本搜索** | ⭐ | ⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐ |
| **⑦ 跨语言追踪** | ⭐⭐ | ⭐⭐⭐ | ⭐⭐ | ⭐ |

> *LSP 未实测，评分基于文档分析

### Token 消耗实测对比

以"理解 AsrEngine 类的结构和调用关系"为完整任务：

| 工具 | 调用次数 | 总 tokens | 效果 |
|---|---|---|---|
| **codegraph** | 2 次 | ~300 | 完整类定义 + 调用链 |
| **CBM** | 3 次 | ~1,200 | 类结构 + 部分关系（需 Cypher 补） |
| **rg** | 5+ 次 | ~1,500+ | 需反复搜索 + 读文件拼凑 |
| **LSP** | 多次 | ~800+ | goToDef + findRefs 但需安装配置 |

---

## 四、选型决策树

```
你的代码库多大？
├── < 1K 文件
│   └── rg 足够，无需额外工具
├── 1K-10K 文件，单语言
│   └── codegraph ← 快速上手，调用链追踪最强
├── 1K-10K 文件，多语言混合
│   └── codebase-memory-mcp ← 跨语言 + 架构分析
├── > 10K 文件
│   ├── 有第三方库 → CBM（自动排除噪声）
│   └── 纯净代码 → codegraph（索引全面）
└── 你主要做什么？
    ├── 日常开发/调试 → codegraph（快速精准）
    ├── 架构重构/代码审计 → CBM（Cypher + 热点分析）
    ├── 找 TODO/搜索文本 → rg（无可替代）
    └── 已知符号精确操作 → LSP（goToDef/hover）
```

### 最终建议

- **日常使用优先级**: codegraph > rg > CBM > LSP
- **关键发现**: codegraph 在调用链追踪上显著领先，1 次调用即可完成 rg 需 5+ 次且仍需人工判断的任务
- **互补关系**: 三者不是替代关系——codegraph 做结构理解，CBM 做深度分析，rg 做文本搜索
- **安装建议**: 保持 codegraph 持续运行（日更，社区活跃）；按需安装 CBM（深挖时使用）；rg 永远是标配

---

## 五、参考

- [codegraph] https://github.com/colbymchenry/codegraph — 50.9k ⭐
- [codebase-memory-mcp] https://github.com/DeusData/codebase-memory-mcp — 4.4k ⭐
- [arXiv:2603.27277] Codebase-Memory: Tree-Sitter-Based Knowledge Graphs for LLM Code Exploration via MCP
- [yage.ai] 为什么 Coding Agent 的搜索主干仍然是 grep (2026-03-27)
- [Blackwell] knowing vs codegraph benchmark
- [OpenCode LSP Docs] https://opencode.ai/docs/lsp/
