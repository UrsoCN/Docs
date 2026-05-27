---
description: "Use when writing AI-generated summaries, updating docs/subpage/Agent_summary/**, or importing notes into Agent_summary. Keeps topic index and docs/_sidebar.md synchronized with new summary articles."
name: "Agent Summary Navigation"
applyTo: "docs/subpage/Agent_summary/**/*.md"
---

# Agent Summary Navigation

- `docs/subpage/Agent_summary/_Agent_summary.md` is the topic index page for AI-generated summary articles.
- Keep the intro blockquote at the top unless the user asks to replace it.
- Keep a self-link entry: `[Agent_summary](./_Agent_summary.md)`.
- Add each summary article under that self-link using file-relative links such as `./subsubpage/article.md`.
- When adding a new summary article under `docs/subpage/Agent_summary/subsubpage/`, rerun `node scripts/generate-sidebar.js` in the same change.
- If a title is user-facing but the filename needs cleanup, keep the label readable and the filename URL-safe.
- Do not leave Agent_summary articles orphaned from navigation.