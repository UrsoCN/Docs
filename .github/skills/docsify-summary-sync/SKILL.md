---
name: docsify-summary-sync
description: 'Summarize notes or source material into this docsify repository and keep navigation synchronized. Use when creating AI summaries, importing notes from Obsidian MCP, updating Agent_summary, adding topic index links, or updating docs/_sidebar.md and docs/subpage/*/_*.md together.'
argument-hint: 'Source note or topic to summarize, target topic, and desired title'
user-invocable: true
---

# Docsify Summary Sync

Use this skill when adding a new summary article so the article content and the docsify navigation stay consistent in one change.

## When To Use

- Summarizing notes from Obsidian MCP into `docs/`
- Adding a new article under `docs/subpage/*/subsubpage/`
- Updating `docs/subpage/<Topic>/_<Topic>.md` after creating a new article
- Regenerating `docs/_sidebar.md` so the new article is reachable from root navigation
- Writing into `docs/subpage/Agent_summary/`

## Required Output Files

For a new summary article, update all relevant files in the same change:

1. The article page under `docs/subpage/<Topic>/subsubpage/`
2. The topic index page `docs/subpage/<Topic>/_<Topic>.md`
3. The generated root sidebar `docs/_sidebar.md`

If the topic does not exist yet, create the topic folder and index page first, then rerun `node scripts/generate-sidebar.js`.

## Docsify Link Rules

- In `docs/_sidebar.md`, keep standard relative links such as `./subpage/Agent_summary/_Agent_summary.md`.
- The custom link renderer in `docs/index.html` rewrites rendered root-navigation links that start with `./subpage/` into absolute docsify routes.
- In topic index pages such as `docs/subpage/Agent_summary/_Agent_summary.md`, use file-relative links such as `./subsubpage/article.md`.
- In article pages under `docs/subpage/**/subsubpage/`, use file-relative links and image paths.
- Keep filenames URL-safe. Prefer underscores or ASCII replacements instead of spaces or `#`.

## Procedure

1. Identify the target topic folder.
2. Read the target topic index page before editing.
3. Create or update the article page under `subsubpage/`.
4. Add or update the self-link and child article entry in the topic index page.
5. Rerun `node scripts/generate-sidebar.js`.
6. Keep labels human-readable, but keep filenames stable and URL-safe.
7. Validate that local Markdown links still resolve after the change.

## Agent_summary Conventions

- `docs/subpage/Agent_summary/_Agent_summary.md` should keep a short intro blockquote.
- The page should include a self-link entry like `[Agent_summary](./_Agent_summary.md)`.
- Each summary article should appear as a child entry under that self-link.
- The same article should also appear under the `Agent_summary` section in `docs/_sidebar.md` after rerunning `node scripts/generate-sidebar.js`.

## Obsidian MCP Notes

- If the source material lives in Obsidian, use Obsidian MCP tools to read the note first.
- Convert the note into a publishable Markdown article, then update docsify navigation in the same task.
- Do not stop after generating only the article body; navigation updates are part of the workflow.