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

## Useful References

- Original docsify setup notes: [docs/subpage/diary/subsubpage/关于使用docsify创建一个静态博客.md](docs/subpage/diary/subsubpage/关于使用docsify创建一个静态博客.md)
- Known docsify pitfalls in this repo: [docs/subpage/diary/subsubpage/使用docsify应当注意的事情.md](docs/subpage/diary/subsubpage/使用docsify应当注意的事情.md)