---
description: "Use when editing docsify markdown under docs/, especially sidebar links, article links, filenames, images, or GitHub Pages rendering issues."
name: "Docsify Markdown Guidelines"
applyTo: "docs/**/*.md"
---

# Docsify Markdown Guidelines

- First classify the file you are editing:
  - `docs/_sidebar.md` is root navigation loaded from the `docs/` directory.
  - Topic index files such as `docs/subpage/AOSP/_AOSP.md` are content pages.
  - Article pages live under `docs/subpage/*/subsubpage/`.
- Current repo state: `docs/index.html` enables `window.$docsify.relativePath` and aliases all `_sidebar.md` requests back to `docs/_sidebar.md`.
- In root-level docs files under `docs/`, keep links relative to the `docs/` directory, except for `docs/_sidebar.md`.
- In `docs/_sidebar.md`, keep standard relative links such as `[AOSP](./subpage/AOSP/_AOSP.md)`.
- `docs/index.html` rewrites `./subpage/...` links from rendered root navigation into absolute docsify routes, so do not manually convert those sidebar links to slash-prefixed paths.
- In topic index files and article pages under `docs/subpage/**`, prefer file-relative links, for example `[刷机](./subsubpage/刷机.md)`.
- Do not reintroduce docs-root-expanded links such as `./subpage/AOSP/...` inside files that already live in `docs/subpage/AOSP/`.
- Keep images file-relative to the current page, usually `images/...`.
- Prefer URL-safe filenames. If you rename a Markdown file to remove spaces or special characters, update every incoming link in the same change.