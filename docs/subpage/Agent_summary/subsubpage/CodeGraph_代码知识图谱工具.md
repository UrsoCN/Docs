---
created: 2026-06-05
modified: 2026-06-05
tags:
  - academic
  - ai
  - ai-summary
  - article
  - javascript
  - linux
---
# CodeGraph: AI 代码知识图谱工具

> CodeGraph 是一个开源工具，为代码库构建预索引的知识图谱，让 AI 编程助手能够以更少的 token 消耗和工具调用理解代码结构。由 colbymchenry 开发，MIT 协议开源。

## 基本信息

- **GitHub**: [colbymchenry/codegraph](https://github.com/colbymchenry/codegraph)
- **Stars**: 40.8k ⭐
- **协议**: MIT
- **语言**: TypeScript / Node.js
- **安装方式**: CLI 自包含二进制（无需 Node.js）或 npm 全局安装
- **支持平台**: macOS / Linux / Windows

## 核心概念

CodeGraph 在项目根目录创建 `.codegraph/` 索引目录，通过静态分析（基于 tree-sitter）提取代码中的符号关系、调用图、类型引用、框架路由等信息，存储在 SQLite 数据库中。AI 代理（如 Claude Code、Cursor、Codex、Hermes Agent 等）通过 MCP 协议查询这个索引，而不是用 grep/read 逐文件扫描。

整个过程 **100% 本地**，不离开你的机器。

## 主要功能

| 功能 | 说明 |
|------|------|
| **符号搜索** | 按名称跨代码库查找符号（函数、类、变量等） |
| **影响分析** | 追踪某符号的调用者/被调用者，分析改动波及范围 |
| **调用链追踪** | 追踪两个符号之间的完整调用路径 |
| **代码上下文** | 一次调用组合搜索+节点+调用者+被调用者 |
| **智能探索** | 返回多个相关符号的源码，按文件分组 + 关系图 |
| **文件索引** | 比文件系统扫描更快的目录结构查询 |
| **路由感知** | 识别 Django/Flask/FastAPI/Express/NestJS/Spring 等 14+ 框架的路由绑定 |
| **混合语言桥接** | Swift↔ObjC、React Native 原生桥、TurboModules、Fabric views 等跨语言关联 |
| **自动同步** | 原生文件系统事件监听（FSEvents/inotify/ReadDirectoryChangesW），编辑后自动重索引 |

## MCP 支持情况

**有！CodeGraph 本身就是一个 MCP（Model Context Protocol）服务器。** 安装后运行 `codegraph serve --mcp` 启动 MCP 服务，提供以下 **10 个工具**：

| MCP 工具 | 用途 |
|----------|------|
| `codegraph_search` | 按名称跨代码库查找符号 |
| `codegraph_context` | 为任务构建相关代码上下文（搜索+节点+调用者+被调用者组合） |
| `codegraph_trace` | 追踪两个符号间的完整调用路径，支持动态分发 |
| `codegraph_callers` | 查找一个函数被谁调用 |
| `codegraph_callees` | 查找一个函数调用了什么 |
| `codegraph_impact` | 分析修改某符号会影响哪些代码 |
| `codegraph_node` | 获取特定符号详情（可选含源码） |
| `codegraph_explore` | 返回多个相关符号按文件分组的源码 + 关系图 |
| `codegraph_files` | 获取索引的文件结构（比文件系统扫描更快） |
| `codegraph_status` | 检查索引健康状态和统计信息 |

## 性能和基准

在 7 个真实开源代码库上的测试结果（中位数，4 次运行）：

- **平均节省**: 16% 更便宜 · 47% 更少 token · 58% 更少工具调用 · 22% 更快
- **VS Code** (~10k 文件): 81% 更少工具调用, 64% 更少 token
- **Django** (~3k 文件): 77% 更少工具调用, 60% 更少 token
- **Tokio** (~790 文件): 57% 更少工具调用, 38% 更少 token

核心原因：有索引时，AI 代理一次 `codegraph_explore` 就能得到答案（通常零文件读取）；没有时，代理需多次 grep/find/read 才能找到相关代码。

## 支持的 AI 代理

Claude Code、Cursor、Codex CLI、OpenCode、**Hermes Agent**、Gemini CLI、Antigravity IDE、Kiro — `codegraph install` 自动检测并配置这些工具。

## 快速开始

```bash
# 1. 安装 CLI（无需 Node.js）
curl -fsSL https://raw.githubusercontent.com/colbymchenry/codegraph/main/install.sh | sh

# 2. 连接到你的 AI 代理
codegraph install

# 3. 在项目中创建索引
cd your-project
codegraph init -i
```

之后你的 AI 代理就能自动使用 CodeGraph 的 MCP 工具来查询代码了。

## 总结

CodeGraph 是一个**有完整 MCP 支持**的代码知识图谱工具。它的价值在于：

- **对 AI 开发者**: 大幅提升 AI 代理理解代码的效率，节省 token 和 API 费用
- **对 Hermes 用户**: 可以直接通过 Hermes 的 MCP 客户端集成，让代理更快地理解代码库
- **对团队**: 无需外部服务，100% 本地，配置简单一条命令

它不是一个独立的 MCP 服务器包，而是通过 `codegraph serve --mcp` 运行的完整工具链。同时它也原生支持自动安装到 Hermes Agent 等主流 AI 工具中。
