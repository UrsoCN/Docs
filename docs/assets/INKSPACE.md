---
tags: [type/howto, topic/docsify, topic/dev-tools]
---

# Inkspace · Inkspace

为 Urso 的笔记设计的原创 Docsify 5 主题。暖纸色日间与墨绿灰夜间，共用松绿色强调色、书页式标题和清晰的中文正文排版。

页顶的日间／夜间按钮可以随时切换。首次访问跟随系统外观，手动选择后记住偏好；刷新、切换文章和再次打开页面都会保留选择。同一站点的其他标签页也会同步。

手机上的目录按钮位于页顶，正文右侧的目录仅在宽屏显示。代码、公式、表格和标注随模式配色；Mermaid 图表在切换时重新绘制。照片和外部 PlantUML 图片保留原始颜色。

## 调整外观

主题依赖 Docsify 5.1.0 的核心布局，站点入口位于 `docs/index.html`。文件职责如下：

| 文件 | 用途 |
| --- | --- |
| `docs/assets/site.css` | 配色变量、中文排版、封面、导航和响应式布局 |
| `docs/assets/inkspace.js` | 首屏主题选择、偏好记忆、系统外观监听与手机目录按钮 |
| `docs/assets/inkspace-mermaid.js` | 图表配色、主题切换后重新渲染 |
| `docs/assets/obsidian-callouts.css` | 标注的基础样式，主题文件补充夜间配色 |

修改 `site.css` 顶部的 `--ink-*` 变量即可调整色板。`:root` 是日间配色，`:root[data-theme="dark"]` 是夜间配色。图表的对应颜色位于 `inkspace-mermaid.js` 的 `themeVariables` 中。

主题使用本机字体，不加载在线字体。中文标题优先使用思源宋体或系统宋体，正文优先使用思源黑体或系统无衬线字体，代码优先使用 Cascadia Code。

如需恢复跟随系统，在浏览器开发者工具中执行以下代码，再刷新页面：

```js
localStorage.removeItem('urso-inkspace-theme');
location.reload();
```

## 本地预览

```bash
docsify serve docs/
```

修改后分别检查日间、夜间与手机宽度。尤其留意长标题、公式横向滚动、代码复制、搜索结果，以及带 Mermaid 图表的页面在反复切换模式后是否仍完整。
