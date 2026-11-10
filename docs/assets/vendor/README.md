# Docsify compatibility patches

These browser assets are checked in because their published versions do not work
unchanged with Docsify 5.1.0. Keep the source readable so updates can be reviewed.

| File | Upstream | Local changes |
| --- | --- | --- |
| `docsify-search-5.1.0.js` | https://cdn.jsdelivr.net/npm/docsify@5.1.0/dist/plugins/search.js | Add the missing `markdownToTxtRenderer.def()` method required by Marked 18. Reference definitions have no visible text. Without it, indexing the quoted Kati/Soong definitions in `Android编译系统-Android.mk篇.md` aborts with `this.renderer.def is not a function`. |
| `docsify-plugin-toc-1.3.1.js` | https://unpkg.com/docsify-plugin-toc@1.3.1/dist/docsify-plugin-toc.js | Replace removed `Docsify.dom.toggleClass` calls with `classList`; scope every heading selector to `#main`; exclude quoted/callout headings; align scroll highlighting with the configured heading depth; label the TOC landmark. Layout uses `../site.css`. |
| `docsify-latex-0.5.2.js` | https://cdn.jsdelivr.net/npm/docsify-latex@0.5.2/dist/docsify-latex.js | Snapshot formula nodes in `#main`; read TeX from `textContent` and mark rendered KaTeX nodes to prevent parsing generated HTML again; use Docsify's callback hook to finish math before the TOC copies headings. The upstream async hook yields while iterating a live collection: inserting a TOC formula shifts that collection and renders the first body formula twice. |

Licenses are retained beside the files. When upgrading, compare against the exact
upstream sources and remove a local patch only after its regression is resolved.

After changing these assets, preview with `docsify serve docs/` and check:

- A fresh search index including the Android.mk note, then search for Transformer.
- A deep article's sidebar links and relative PDF URL under a deployment subdirectory.
- Tables, inline/display math, Mermaid, PlantUML, copy code, and image zoom.
- The RNN-T lesson's first CTC display formula and its inline formula in the right TOC (`前向概率`), including navigation away/back and light/dark switches. There should be no `.katex-error` or unrendered `docsify-latex` nodes.
- Callouts with no title, custom titles, `+`/`-` folding, and headings inside quotes.
- Nested sidebar toggles, right TOC anchors, and the mobile sidebar without overflow.
