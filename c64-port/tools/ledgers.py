#!/usr/bin/env python3
"""Rebuild the shared ledger indices from per-milestone files.

README.md, docs/HANDOFF.md, docs/ROADMAP.md, docs/VERIFICATION.md and
docs/SOURCE_MAP.md are generated. Never edit them by hand: each milestone owns
docs/milestones/<id>.md, and this tool assembles the indices from
docs/ledgers/<NAME>.md (the frozen pre-split text, where
`<!-- milestone: <id> -->` lines stand for a legacy milestone's section)
followed by every non-legacy milestone ordered by (date, id).

  python3 tools/ledgers.py          # rewrite the indices
  python3 tools/ledgers.py --check  # exit 1 if an index is stale or a milestone file is invalid
  python3 tools/ledgers.py --validate  # only check the milestone files (lane PRs leave indices stale)
"""
import argparse
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
MILESTONES = ROOT / 'docs/milestones'
TEMPLATES = ROOT / 'docs/ledgers'
LEDGERS = {
    'README': 'README.md',
    'HANDOFF': 'docs/HANDOFF.md',
    'ROADMAP': 'docs/ROADMAP.md',
    'VERIFICATION': 'docs/VERIFICATION.md',
    'SOURCE_MAP': 'docs/SOURCE_MAP.md',
}
ID = re.compile(r'^[a-z0-9][a-z0-9-]*$')
DATE = re.compile(r'^\d{4}-\d{2}-\d{2}(T\d{2}:\d{2}(:\d{2})?Z)?$')
MARKER = re.compile(r'(?m)^<!-- milestone: ([a-z0-9-]+) -->\n')


def parse(path):
    text = path.read_text()
    if not text.startswith('---\n') or '\n---\n' not in text[3:]:
        raise ValueError(f'{path.name}: missing front matter')
    head, body = text[4:].split('\n---\n', 1)
    meta = {}
    for line in head.splitlines():
        key, sep, value = line.partition(': ')
        if not sep:
            raise ValueError(f'{path.name}: bad front matter line {line!r}')
        meta[key] = value
    meta['body'] = body
    errors = []
    if meta.get('id') != path.stem or not ID.match(path.stem):
        errors.append('id must match the file name and use [a-z0-9-]')
    if not meta.get('title') or '\n' in meta['title']:
        errors.append('title is required')
    if 'seq' in meta:
        if not path.stem.startswith('legacy-') or not meta['seq'].isdigit():
            errors.append('seq is reserved for migrated legacy milestones')
    elif path.stem.startswith('legacy-'):
        errors.append('the legacy- prefix is reserved for migrated milestones')
    elif not DATE.match(meta.get('date', '')):
        errors.append('date is required (YYYY-MM-DD or YYYY-MM-DDTHH:MMZ)')
    if errors:
        raise ValueError(f'{path.name}: ' + '; '.join(errors))
    return meta


def load():
    milestones = {}
    for path in sorted(MILESTONES.glob('*.md')):
        milestones[path.stem] = parse(path)
    return milestones


def section(meta):
    return f"## {meta['title']}\n\n{meta['body'].strip(chr(10))}\n"


def render(name, milestones):
    template = (TEMPLATES / f'{name}.md').read_text()

    def legacy(match):
        meta = milestones.get(match.group(1))
        if meta is None or 'seq' not in meta:
            raise ValueError(f'{name}: unknown legacy milestone {match.group(1)}')
        return f"## {meta['title']}\n{meta['body']}"

    text = MARKER.sub(legacy, template)
    fresh = sorted((m for m in milestones.values() if 'seq' not in m),
                   key=lambda m: (m['date'], m['id']))
    for meta in fresh:
        if not text.endswith('\n\n'):
            text += '\n' if text.endswith('\n') else '\n\n'
        text += section(meta)
    return text


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--check', action='store_true')
    parser.add_argument('--validate', action='store_true')
    args = parser.parse_args()
    try:
        milestones = load()
        outputs = {path: render(name, milestones) for name, path in LEDGERS.items()}
    except (OSError, ValueError) as error:
        print('FAIL:', error, file=sys.stderr)
        return 1
    if args.validate:
        print(f'PASS: {len(milestones)} milestone files render into {len(outputs)} indices')
        return 0
    stale = [path for path, text in outputs.items()
             if not (ROOT / path).is_file() or (ROOT / path).read_text() != text]
    if args.check:
        for path in stale:
            print(f'STALE: {path} (run python3 tools/ledgers.py)', file=sys.stderr)
        if not stale:
            print(f'PASS: {len(outputs)} indices match {len(milestones)} milestones')
        return 1 if stale else 0
    for path in stale:
        (ROOT / path).write_text(outputs[path])
        print('wrote', path)
    return 0


if __name__ == '__main__':
    sys.exit(main())
