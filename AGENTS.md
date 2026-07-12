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
- Before changing Markdown links, check `docs/index.html` for `window.$docsify.relativePath`.
- Current repo state: `relativePath` is enabled, so topic landing pages and article pages under `docs/subpage/**` should use file-relative links.
- Root docs files such as `docs/README.md` and `docs/_coverpage.md` can keep links relative to the `docs/` directory.
- `docs/_sidebar.md` is a special case: keep normal relative links such as `./subpage/AOSP/_AOSP.md`, and let the custom docsify link renderer in `docs/index.html` rewrite them to absolute routes at render time.
- Do not hand-maintain `docs/_sidebar.md`; rerun `node scripts/generate-sidebar.js` instead.
- Do not partially mix link schemes during a cleanup. For `docs/subpage/**`, prefer local file-relative links like `./subsubpage/article.md`.
- Keep `docs/_sidebar.md` aligned with the root navigation file loaded via the alias configured in `docs/index.html`.

## Tag Convention

All notes should use a consistent three-category tag system. Tags go in the YAML frontmatter `tags:` field.

### Categories

Each note gets **3–5 tags** total:

| Category | Format | Purpose | Examples |
|----------|--------|---------|---------|
| Type (1) | `type/xxx` | What kind of note | `type/article`, `type/moc`, `type/howto`, `type/diary`, `type/survey`, `type/comparison` |
| Status (optional) | `status/xxx` | How mature it is | `status/draft`, `status/evergreen` |
| Topic(s) (1–3) | `lang/xxx` or `topic/xxx` | What it's about | `lang/rust`, `topic/embedded`, `topic/ai` |

### Naming Rules

- **All lowercase**, kebab-case (`topic/embedded-linux`, not `topic/embeddedLinux`)
- **Singular** (`type/article` not `type/articles`)
- **Max 2 levels deep** (`lang/rust` ✅, `topic/embedded-linux` ✅, `topic/programming/languages/rust` ❌)
- **Namespaces**: `lang/` for programming languages, `topic/` for everything else, `type/` for note classification

### Current Tag Inventory

After the migration (July 2026):

```
Type tags:
  type/article (64), type/moc (10), type/learning (4),
  type/survey (1), type/comparison (1)

Language tags:
  lang/python (12), lang/cpp (10), lang/rust (5),
  lang/csharp (3), lang/javascript (3)

Topic tags:
  topic/linux (20), topic/docsify (13), topic/embedded-linux (12),
  topic/embedded (10), topic/ai (9), topic/aosp (8),
  topic/ml (8), topic/cuda (6), topic/esp32 (5), topic/wsl (5),
  topic/testing (4), topic/xvf3800 (4), topic/academic (3),
  topic/cv (3), topic/graphics (3), topic/interview (3),
  topic/obsidian (3), topic/opengl (3), topic/bluetooth (2),
  topic/latex (2), topic/robotics (2), topic/android-flashing (2),
  topic/audio (1), topic/dev-tools (1)
```

### Granularity Rules of Thumb

Good tags sit in the **5–30 note** sweet spot:
- **≤2 notes**: too specific → delete the tag, use `[[wikilinks]]` or full-text search instead
- **≥50 notes**: too broad → split into more specific tags
- **New topic tags** are fine at 1 note initially; they'll grow as related notes accumulate

Only add a specific sub-tag (e.g. `topic/tokio` instead of just `lang/rust`) when you have **≥3 notes** that warrant it.

### Examples

```yaml
# A Rust tutorial note
---
tags: [type/article, lang/rust, topic/embedded]
---

# Topic landing page (Map of Content)
---
tags: [type/moc, topic/aosp, topic/linux]
---

# Personal journal entry
---
tags: [type/diary]
---
```

### Migration Note

The vault was migrated from flat tags to this convention in July 2026. The migration script is at `/tmp/migrate_tags.py`. Do not re-run it — future tags should be added directly following this convention.

## Useful References

- Original docsify setup notes: [docs/subpage/diary/subsubpage/关于使用docsify创建一个静态博客.md](docs/subpage/diary/subsubpage/文档整理_随笔/关于使用docsify创建一个静态博客.md)
- Known docsify pitfalls in this repo: [docs/subpage/diary/subsubpage/使用docsify应当注意的事情.md](docs/subpage/diary/subsubpage/文档整理_随笔/使用docsify应当注意的事情.md)