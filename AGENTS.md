# Project Guidelines

## Project Shape

- This repository is a personal docsify knowledge base published from `docs/` to GitHub Pages.
- The docsify entrypoint and plugins live in `docs/index.html`.
- Primary navigation is emitted to `docs/_sidebar.md` by `scripts/generate-sidebar.js` from the `docs/subpage/` tree.
- Topic landing pages live under `docs/subpage/<Topic>/_<Topic>.md`.
- Articles usually live under `docs/subpage/<Topic>/subsubpage/`.

## Local Preview

- Use `docsify serve docs/` to preview the site locally.
- Regenerate the root sidebar with `node scripts/generate-sidebar.js` after adding, renaming, or deleting docs under `docs/subpage/`.
- If `docsify` is not installed, install `docsify-cli` first.
- There is no package manifest or automated test suite in this repo; validate documentation changes by previewing the affected route in docsify.

## Documentation Conventions

- Keep filenames URL-safe. Avoid spaces and special characters such as `#`; prefer underscores or ASCII replacements when renaming files.
- Keep article images near the article in a local `images/` directory and link them with file-relative paths.
- Write LaTeX with `$...$` inline on a single line and `$$` as a display block on its own line with a blank line before and after (fenced `tex` also renders, but prefer `$$`), and keep prices and shell variables in backticks since `$` only delimits math.
- Before changing Markdown links, check `docs/index.html` for `window.$docsify.relativePath`.
- Current repo state: `relativePath` is enabled, so topic landing pages and article pages under `docs/subpage/**` should use file-relative links.
- Root docs files such as `docs/README.md` and `docs/_coverpage.md` can keep links relative to the `docs/` directory.
- `docs/_sidebar.md` is a special case: keep normal relative links such as `./subpage/AOSP/_AOSP.md`, and let the custom docsify link renderer in `docs/index.html` rewrite them to absolute routes at render time.
- Do not hand-maintain `docs/_sidebar.md`; rerun `node scripts/generate-sidebar.js` instead.
- Do not partially mix link schemes during a cleanup. For `docs/subpage/**`, prefer local file-relative links like `./subsubpage/article.md`.
- Keep `docs/_sidebar.md` aligned with the root navigation file loaded via the alias configured in `docs/index.html`.

## Callouts

Note asides use Obsidian callout syntax, rendered as styled boxes by `docs/assets/obsidian-callouts.js` + `docs/assets/obsidian-callouts.css`.

```markdown
> [!warning] Title is optional
> Body text — every line, blank ones included, starts with `> `.
```

- `[!type]` goes first, right after `> `; a line without `>` breaks out of the box.
- Use a bare `>` line before lists and code inside a callout.
- 13 Obsidian types plus `important` (alias of `tip`); full alias table in `docs/assets/obsidian-callouts.js`.
- Fold with `[!note]-` (collapsed) or `+` (expanded).
- Asides only (caveats, prerequisites, summaries). Never nest; never wrap body text.

## Tag Convention

Tags go in the YAML frontmatter `tags:` field: exactly one `type/`, one to three `lang/` or `topic/`, and optionally one `status/`.

| Namespace | Purpose | Values |
|-----------|---------|---------------|
| `type/` (required, 1) | Note kind | `article` `learning` `moc` `howto` `survey` `comparison` |
| `status/` (optional) | Maturity | `draft` (reserved: `evergreen`) |
| `lang/` | Programming language | `python` `cpp` `rust` `csharp` `javascript` |
| `topic/` | Everything else | see below |

- Lowercase kebab-case (`topic/embedded-linux`, not `topic/embeddedLinux`); singular (`type/article`).
- Two levels max: `lang/rust` ✅, `topic/programming/languages/rust` ❌.
- Reuse an established tag before inventing a synonym (`topic/speech`, not `topic/audio-ml`). Refresh this list when you add a tag — current `topic/` tags: `ai` `ml` `llm` `speech` `audio` `linux` `embedded` `embedded-linux` `esp32` `xvf3800` `aosp` `robotics` `ros2` `cuda` `cv` `graphics` `opengl` `docsify` `obsidian` `wsl` `testing` `network` `webrtc` `ble` `bluetooth` `ebpf` `performance` `computer-architecture` `academic` `investment` `stock` `interview` `latex` `dev-tools` `android-flashing` `physics`.
- `node scripts/list-tags.js --check` prints the live inventory and flags notes missing a `type/` tag or carrying malformed tags.

### Granularity

- **<3 notes**: reuse the nearest parent tag, or use a `[[wikilink]]` / full-text search instead.
- **>30 notes**: too broad — split it (e.g. `topic/ai` into `llm` / `speech` / `cv`).

3–5 tags is the norm: short notes may carry 2, MOCs often 4–6.

### Examples

```yaml
---
tags: [type/learning, lang/rust, topic/embedded]   # tutorial
---
---
tags: [type/moc, topic/aosp, topic/linux]          # topic landing page (MOC)
---
---
tags: [type/survey, topic/ml, status/draft]        # in-progress survey
---
```

## Useful References

- Original docsify setup notes: [docs/subpage/diary/subsubpage/关于使用docsify创建一个静态博客.md](docs/subpage/diary/subsubpage/文档整理_随笔/关于使用docsify创建一个静态博客.md)
- Known docsify pitfalls in this repo: [docs/subpage/diary/subsubpage/使用docsify应当注意的事情.md](docs/subpage/diary/subsubpage/文档整理_随笔/使用docsify应当注意的事情.md)
