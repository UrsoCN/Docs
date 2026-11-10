// Print the live tag inventory and report tag-convention violations.
// Usage: node scripts/list-tags.js [--check]
const fs = require('fs');
const path = require('path');

const repoRoot = path.resolve(__dirname, '..');
const docsRoot = path.join(repoRoot, 'docs');

const ignoredDirs = new Set(['.git', '.obsidian', 'node_modules', 'images']);
const tagPattern = /^(type|status|lang|topic)\/[a-z0-9]+(?:-[a-z0-9]+)*$/;

function toPosix(filePath) {
  return filePath.split(path.sep).join('/');
}

function listMarkdownFiles(dirPath, out = []) {
  for (const entry of fs.readdirSync(dirPath, { withFileTypes: true })) {
    if (ignoredDirs.has(entry.name)) continue;
    const fullPath = path.join(dirPath, entry.name);
    if (entry.isDirectory()) listMarkdownFiles(fullPath, out);
    else if (entry.isFile() && path.extname(entry.name) === '.md') out.push(fullPath);
  }
  return out;
}

function readTags(filePath) {
  const content = fs.readFileSync(filePath, 'utf8');
  const frontmatter = content.match(/^---\r?\n([\s\S]*?)\r?\n---/);
  if (!frontmatter) return null;

  const lines = frontmatter[1].split(/\r?\n/);
  const start = lines.findIndex((line) => /^tags:\s*/.test(line));
  if (start === -1) return [];

  const inline = lines[start].replace(/^tags:\s*/, '').trim();
  if (inline.startsWith('[')) {
    return inline.replace(/^\[|\]$/g, '').split(',')
      .map((tag) => tag.trim().replace(/^['"]|['"]$/g, ''))
      .filter(Boolean);
  }

  const tags = [];
  for (const line of lines.slice(start + 1)) {
    const item = line.match(/^\s*-\s+(.+?)\s*$/);
    if (item) tags.push(item[1].replace(/^['"]|['"]$/g, ''));
    else if (line.trim()) break;
  }
  return tags;
}

const counts = new Map();
const problems = [];
let tagged = 0;

for (const filePath of listMarkdownFiles(docsRoot)) {
  const tags = readTags(filePath);
  if (tags === null) continue;
  const relative = toPosix(path.relative(repoRoot, filePath));
  if (tags.length) tagged += 1;

  for (const tag of tags) counts.set(tag, (counts.get(tag) || 0) + 1);
  if (!tags.some((tag) => tag.startsWith('type/'))) problems.push(`${relative}: missing type/ tag`);
  for (const tag of tags) {
    if (!tagPattern.test(tag)) problems.push(`${relative}: invalid tag "${tag}"`);
  }
}

const groups = { type: [], status: [], lang: [], topic: [], other: [] };
for (const [tag, count] of counts) {
  const namespace = tag.includes('/') ? tag.split('/')[0] : 'other';
  (groups[namespace] || groups.other).push([tag, count]);
}

for (const namespace of ['type', 'status', 'lang', 'topic', 'other']) {
  const entries = groups[namespace];
  if (!entries.length) continue;
  entries.sort((left, right) => right[1] - left[1] || left[0].localeCompare(right[0]));
  console.log(`${namespace}/ (${entries.length})`);
  for (const [tag, count] of entries) console.log(`  ${String(count).padStart(3)}  ${tag}`);
}

console.log(`\n${tagged} tagged notes, ${counts.size} distinct tags, ${problems.length} problems`);

const check = process.argv.includes('--check');
if (check && problems.length) {
  for (const problem of problems) console.error(problem);
  process.exitCode = 1;
}
