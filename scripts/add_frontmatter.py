#!/usr/bin/env python3
"""
Add YAML frontmatter to all docsify markdown files.
Uses git commit history for created/modified timestamps.
Generates tags from folder path + content keyword analysis.
"""
import os
import re
import subprocess
import json
from datetime import datetime

DOCS_DIR = os.path.expanduser("/home/u/Docs/docs")
SKIP_FILES = {"_sidebar.md", "_coverpage.md", "README.md"}
SKIP_PATTERNS = [r"^_", r"README"]

def git_cmd(cmd):
    """Run a git command in the Docs repo and return stdout."""
    result = subprocess.run(
        cmd,
        cwd=os.path.dirname(DOCS_DIR),
        capture_output=True,
        text=True,
        timeout=30
    )
    return result.stdout.strip()

def get_git_dates(filepath):
    """Get created and last-modified dates from git history.
    Returns (created_iso, modified_iso) or (None, None) if not in git."""
    rel = os.path.relpath(filepath, os.path.dirname(DOCS_DIR))
    
    # Get first commit date (created)
    created = git_cmd([
        "git", "log", "--diff-filter=A", "--follow",
        "--format=%aI", "--", rel
    ])
    if not created:
        created = git_cmd([
            "git", "log", "--follow",
            "--format=%aI", "--", rel
        ])
    created = created.split("\n")[-1] if created else None
    
    # Get last commit date (modified)
    modified = git_cmd([
        "git", "log", "-1", "--format=%aI", "--", rel
    ])
    modified = modified if modified else None
    
    return created, modified

def extract_tags_from_content(content, folder_path, filepath):
    """Generate tags based on folder location and content keywords."""
    tags = []
    
    # -- Folder-based primary tag --
    # Normalize: skip 'subpage'/'subsubpage' virtual dirs, get the actual topic folder
    folder_parts = folder_path.replace("docs/subpage/", "").split("/")
    # Remove known structural prefixes
    folder_parts = [p for p in folder_parts if p not in ("subpage", "subsubpage", "")]
    primary_tag = folder_parts[0] if folder_parts else "uncategorized"
    
    # Normalize topic name to a tag
    tag_map = {
        "Agent_summary": "ai-summary",
        "AOSP": "aosp",
        "C&C++": "cpp",
        "Csharp": "csharp",
        "Rust": "rust",
        "MachineLearning": "machine-learning",
        "OpenGL": "opengl",
        "WSL": "wsl",
        "Linux&嵌入式": "linux-embedded",
        "diary": None,  # diary gets content-based tags, not a blanket "diary" tag
    }
    mapped = tag_map.get(primary_tag)
    if mapped:
        tags.append(mapped)
    elif primary_tag not in ("diary",):
        tags.append(primary_tag.lower())
    
    # -- Content-based keywords (only check title + first 500 chars to avoid false positives) --
    content_lower = content[:500].lower()
    
    # Tech / programming keywords
    if any(k in content_lower for k in ["python", "jupyter", "conda", "numpy", "tensor", "pytorch"]):
        tags.append("python")
    if any(k in content_lower for k in ["javascript", "node", "npm", "vue"]):
        tags.append("javascript")
    if any(k in content_lower for k in ["docker", "container"]):
        tags.append("docker")
    if any(k in content_lower for k in ["cuda", "gpu", "nvidia"]):
        tags.append("cuda")
    if any(k in content_lower for k in ["latex", "mathjax", "katex", "公式"]):
        tags.append("latex")
    if any(k in content_lower for k in ["blog", "docsify", "github pages", "静态博客"]):
        tags.append("docsify")
    if any(k in content_lower for k in ["刷机", "fastboot", "adb", "bootloader"]):
        tags.append("android-flashing")
    if any(k in content_lower for k in ["目标检测", "object detection", "yolo"]):
        tags.append("computer-vision")
    if any(k in content_lower for k in ["面试", "华为od", "od机试"]):
        tags.append("interview")
    if any(k in content_lower for k in ["嵌入式", "stm32", "arm", "单片机"]):
        tags.append("embedded")
    if any(k in content_lower for k in ["操作系统", "linux", "kernel", "ubuntu"]):
        tags.append("linux")
    if any(k in content_lower for k in ["jabref", "引用", "参考文献", "citation"]):
        tags.append("academic")
    if any(k in content_lower for k in ["bluetooth", "蓝牙", "sbc"]):
        tags.append("bluetooth")
    if any(k in content_lower for k in ["cts", "兼容性"]):
        tags.append("testing")
    if any(k in content_lower for k in ["股票", "基金", "投资", "指数"]):
        tags.append("investment")
    if any(k in content_lower for k in ["ai ", "llm", "asr", "qwen", "agent ", "codegraph"]):
        tags.append("ai")
    if any(k in content_lower for k in ["opengl", "shader", "glfw", "glew"]):
        tags.append("graphics")
    if any(k in content_lower for k in ["电磁场", "maxwell", "仿真"]):
        tags.append("physics")
    if any(k in content_lower for k in ["swift", "objective-c"]):
        tags.append("ios")
    if any(k in content_lower for k in ["obsidian"]):
        tags.append("obsidian")
    if any(k in content_lower for k in ["ci/cd", "github actions"]):
        tags.append("ci-cd")
    
    # For topic index files, add a "topic-index" tag
    filename = os.path.basename(filepath)
    if filename.startswith("_") and filename.endswith(".md"):
        tags.append("topic-index")
    else:
        tags.append("article")
    
    # Deduplicate and sort
    tags = sorted(set(t for t in tags if t))
    return tags

def has_frontmatter(content):
    """Check if content already has YAML frontmatter."""
    return content.startswith("---")

def extract_existing_frontmatter(content):
    """Extract existing frontmatter if present."""
    if not content.startswith("---"):
        return {}, content
    
    parts = content.split("---", 2)
    if len(parts) < 3:
        return {}, content
    
    fm_text = parts[1]
    body = parts[2]
    
    # Try to parse as YAML (simple line-by-line for safety)
    fm = {}
    for line in fm_text.strip().split("\n"):
        if ":" in line:
            key, val = line.split(":", 1)
            key = key.strip()
            val = val.strip()
            # Remove quotes
            val = val.strip("\"'")
            if key in ("tags",):
                # Tags might be a YAML list
                if val.startswith("["):
                    val = [v.strip().strip("'\"") for v in val.strip("[]").split(",")]
                else:
                    val = [val]
            fm[key] = val
    
    return fm, body

def format_frontmatter(fm):
    """Format a dict as YAML frontmatter string."""
    lines = ["---"]
    for key in ["created", "modified", "tags"]:
        if key in fm:
            val = fm[key]
            if key == "tags" and isinstance(val, list):
                lines.append(f"tags:")
                for t in val:
                    lines.append(f"  - {t}")
            else:
                lines.append(f"{key}: {val}")
    lines.append("---")
    return "\n".join(lines)

def process_file(filepath):
    """Add or update frontmatter on a single markdown file."""
    with open(filepath, "r", encoding="utf-8") as f:
        raw = f.read()
    
    existing_fm, body = extract_existing_frontmatter(raw)
    
    # Get dates from git
    created, modified = get_git_dates(filepath)
    if created:
        created_dt = datetime.fromisoformat(created.replace("Z", "+00:00"))
        existing_fm["created"] = created_dt.strftime("%Y-%m-%d")
    if modified:
        modified_dt = datetime.fromisoformat(modified.replace("Z", "+00:00"))
        existing_fm["modified"] = modified_dt.strftime("%Y-%m-%d")
    
    # Determine tags - regenerate from scratch (ignore any existing tags)
    folder_path = os.path.relpath(filepath, DOCS_DIR)
    # Use body (without frontmatter) for content analysis to avoid self-referencing
    content_for_tags = body
    tags = extract_tags_from_content(content_for_tags, folder_path, filepath)
    existing_fm["tags"] = tags
    
    # Build new frontmatter
    new_fm = format_frontmatter(existing_fm)
    
    # Write file
    new_content = new_fm + "\n" + body.lstrip("\n")
    with open(filepath, "w", encoding="utf-8") as f:
        f.write(new_content)
    
    return True

def main():
    # Collect all markdown files
    all_files = []
    for root, dirs, files in os.walk(DOCS_DIR):
        for f in files:
            if not f.endswith(".md"):
                continue
            if f in SKIP_FILES:
                continue
            if f.startswith("_"):
                # Keep topic index files
                pass
            all_files.append(os.path.join(root, f))
    
    all_files.sort()
    print(f"Found {len(all_files)} files to process\n")
    
    success = 0
    errors = []
    
    for fp in all_files:
        try:
            process_file(fp)
            rel = os.path.relpath(fp, DOCS_DIR)
            print(f"  ✓ {rel}")
            success += 1
        except Exception as e:
            rel = os.path.relpath(fp, DOCS_DIR)
            print(f"  ✗ {rel}: {e}")
            errors.append((rel, str(e)))
    
    print(f"\nDone! {success}/{len(all_files)} files processed.")
    if errors:
        print(f"\nErrors ({len(errors)}):")
        for rel, err in errors:
            print(f"  {rel}: {err}")

if __name__ == "__main__":
    main()
