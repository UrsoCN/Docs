#!/usr/bin/env python3
"""Analyze tag relationships across all docsify markdown files."""
import os, json

docs_dir = 'docs/subpage'
tag_map = {}

for root, dirs, files in os.walk(docs_dir):
    for f in files:
        if not f.endswith('.md'):
            continue
        fp = os.path.join(root, f)
        with open(fp, 'r') as fh:
            content = fh.read()
        
        if not content.startswith('---'):
            continue
        parts = content.split('---', 2)
        if len(parts) < 3:
            continue
        
        fm_text = parts[1]
        tags = [l.strip()[2:].strip() for l in fm_text.strip().split('\n') if l.strip().startswith('- ')]
        
        rel = os.path.relpath(fp, docs_dir)
        title = content.split('---', 2)[2].strip().split('\n')[0].strip('# ')
        
        for tag in tags:
            if tag not in ('topic-index', 'article'):
                tag_map.setdefault(tag, []).append(rel)

print(json.dumps(tag_map, ensure_ascii=False, indent=2))
