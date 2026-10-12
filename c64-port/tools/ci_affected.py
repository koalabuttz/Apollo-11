#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Pick the host, sanitizer and C64 targets a change can affect.

Reads make's rule database once and walks each target's prerequisites and
recipe words. A Python recipe script also counts the files it names, so a
target is affected when a changed path (or its file name, for scripts that
build paths from a directory) appears anywhere in that closure. Prints JSON:

  {"full": bool, "test": [...], "sanitize": [...], "c64": [...], "vice": [...]}

`full` is true when the change touches shared build plumbing or would select
more than --limit targets; CI then runs the whole suite instead.

  python3 tools/ci_affected.py --base origin/port/c64-native-c
  python3 tools/ci_affected.py --files src/waitlist.c tools/mk/foo.mk
"""
import argparse
import json
import pathlib
import re
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
SHARED = ('Makefile', 'include/agc_word.h', 'src/agc_word.c', 'tools/ci_affected.py')
IGNORED = re.compile(r'^(docs/|evidence/|README\.md$|CONTRIBUTING-port\.md$|tools/ledgers\.py$|'
                     r'tools/evidence\.py$|tools/new_milestone\.py$|tools/setup_cloud\.sh$|\.gitignore$)')
WORD = re.compile(r'[A-Za-z0-9_./+-]+')
INCLUDE = re.compile(r'^\s*#\s*include\s*"([^"]+)"', re.M)
INCLUDE_DIRS = ('include', 'src', 'tests', 'platform/c64', 'tools')


def database():
    text = subprocess.run(['make', '-pRrq', 'SKIP_EVIDENCE_RESTORE=1', '__ci_affected_none__'],
                          cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL,
                          text=True).stdout
    rules, current = {}, None
    for line in text.split('# Files', 1)[-1].splitlines():
        if line.startswith('\t'):
            if current:
                rules[current][1].append(line[1:])
            continue
        if not line or line.startswith('#'):
            if not line:
                current = None
            continue
        match = re.match(r'^([^\s:=#][^:=]*?):(?!=)\s*(.*)$', line)
        if match and ' ' not in match.group(1):
            current = match.group(1)
            prereqs = match.group(2).replace('|', ' ').split()
            rule = rules.setdefault(current, [set(), []])
            rule[0].update(prereqs)
        else:
            current = None
    return {target: (prereqs, '\n'.join(recipe)) for target, (prereqs, recipe) in rules.items()}


def script_words(path, cache):
    if path not in cache:
        cache[path] = set()
        try:
            cache[path] = set(WORD.findall((ROOT / path).read_text(errors='replace')))
        except OSError:
            pass
    return cache[path]


def headers(path, cache):
    """Headers a C source or header includes, transitively, as repo paths.
    Make rules rarely list them, so without this a change to a shared header
    selects none of the targets that compile it."""
    if path in cache:
        return cache[path]
    cache[path] = found = set()
    try:
        text = (ROOT / path).read_text(errors='replace')
    except OSError:
        return found
    for name in INCLUDE.findall(text):
        for directory in (str(pathlib.PurePosixPath(path).parent),) + INCLUDE_DIRS:
            candidate = str(pathlib.PurePosixPath(directory) / name)
            if (ROOT / candidate).is_file():
                found.add(candidate)
                found |= headers(candidate, cache)
                break
    return found


def closure(target, rules, seen, scripts):
    words = set()
    stack = [target]
    while stack:
        node = stack.pop()
        if node in seen:
            continue
        seen.add(node)
        words.add(node)
        prereqs, recipe = rules.get(node, ((), ''))
        stack.extend(prereqs)
        for word in WORD.findall(recipe):
            words.add(word)
            if word.endswith('.py') and (ROOT / word).is_file():
                words |= script_words(word, scripts)
    includes = scripts.setdefault(None, {})
    for word in list(words):
        if word.endswith(('.c', '.h', '.S')) and (ROOT / word).is_file():
            words |= headers(word, includes)
    return words


def mk_targets(path):
    targets = set()
    try:
        for line in (ROOT / path).read_text().splitlines():
            match = re.match(r'^([A-Za-z0-9_./-]+)\s*:(?!=)', line)
            if match:
                targets.add(match.group(1))
    except OSError:
        pass
    return targets


def changed_files(base):
    out = subprocess.check_output(['git', 'diff', '--name-only', f'{base}...HEAD', '--', '.'],
                                  cwd=ROOT, text=True)
    prefix = 'c64-port/'
    return [line[len(prefix):] if line.startswith(prefix) else line for line in out.splitlines()]


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument('--base')
    group.add_argument('--files', nargs='*')
    parser.add_argument('--limit', type=int, default=40)
    args = parser.parse_args()
    files = args.files if args.files is not None else changed_files(args.base)
    relevant = [f for f in files if not IGNORED.match(f)]
    result = {'files': len(files), 'full': any(f in SHARED for f in relevant),
              'test': [], 'sanitize': [], 'c64': [], 'vice': []}
    if relevant and not result['full']:
        rules = database()
        names = set(relevant) | {pathlib.PurePosixPath(f).name for f in relevant if '/' in f}
        defined = set()
        for f in relevant:
            if f.endswith('.mk'):
                defined |= mk_targets(f)
        scripts = {}
        for group in ('test', 'sanitize', 'c64'):
            roots = sorted(t for t in rules.get(group, (set(), ''))[0] if t != 'all')
            if group == 'c64':
                roots = sorted({t for t in rules if t.startswith('c64-')} | set(roots))
            for target in roots:
                words = closure(target, rules, set(), scripts)
                if target in defined or words & names:
                    result[group].append(target)
        for target in result['c64']:
            vice = 'vice-' + target[len('c64-'):]
            if vice in rules:
                result['vice'].append(vice)
        for target in sorted(defined):
            for group in ('test', 'sanitize', 'c64'):
                if target.startswith(group + '-') and target not in result[group]:
                    result[group].append(target)
        for group in ('test', 'sanitize'):
            core = group + '-core'
            if core in rules and core not in result[group] and \
                    closure(core, rules, set(), scripts) & names:
                result[group].insert(0, core)
        selected = sum(len(result[k]) for k in ('test', 'sanitize', 'c64'))
        result['full'] = selected > args.limit
    json.dump(result, sys.stdout, indent=1)
    print()
    return 0


if __name__ == '__main__':
    sys.exit(main())
