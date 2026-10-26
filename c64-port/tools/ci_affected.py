#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Pick the host, sanitizer and C64 targets a change can affect.

Reads make's rule database once and walks each target's prerequisites and
recipe words. A Python recipe script also counts the files it names, so a
target is affected when a changed path (or its file name, for scripts that
build paths from a directory) appears anywhere in that closure. Prints JSON:

  {"full": bool, "test": [...], "sanitize": [...], "c64": [...], "vice": [...]}

`full` is true when the change touches shared build plumbing or would select
more than --limit targets; CI then runs the whole suite instead. `c64_full`
(absent means `full`) is true only when the C64 list itself is over the limit,
so the c64 job keeps its own list when host targets alone pass the limit.

  python3 tools/ci_affected.py --base origin/port/c64-native-c
  python3 tools/ci_affected.py --files src/waitlist.c tools/mk/foo.mk

With --base, a C, header, assembly or Python file whose code is the same on
both sides (only comments, blank lines or indentation changed) is left out,
so a comment-only PR runs no build or VICE targets. C files that use
__LINE__ or assert still count, since moving lines changes their output.
"""
import argparse
import ast
import json
import pathlib
import re
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
SHARED = ('Makefile', 'include/agc_word.h', 'src/agc_word.c', 'tools/ci_affected.py', 'tools/mk/_ci-split.mk')
IGNORED = re.compile(r'^(docs/(?!flight-store-unplaced\.json$|flight-routine-mct\.json$|flight-interp-mct\.json$)|evidence/|README\.md$|CONTRIBUTING-port\.md$|tools/ledgers\.py$|'
                     r'tools/evidence\.py$|tools/new_milestone\.py$|tools/setup_cloud\.sh$|\.gitignore$)')
WORD = re.compile(r'[A-Za-z0-9_./+-]+')
INCLUDE = re.compile(r'^\s*#\s*include\s*"([^"]+)"', re.M)
INCLUDE_DIRS = ('include', 'src', 'tests', 'platform/c64', 'tools')
# Prerequisites of these aggregates (tools/mk/ci-extra.mk) join the host and
# C64 lists. A ci-watch-<target> rule replaces the target's closure.
EXTRA = {'test': 'ci-extra-host', 'c64': 'ci-extra-c64'}


def database():
    text = subprocess.run(['make', '-pRrq', 'SKIP_EVIDENCE_RESTORE=1', '__ci_affected_none__'],
                          cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL,
                          text=True).stdout
    implicit, files = text.split('# Files', 1) if '# Files' in text else ('', text)
    patterns = []
    # Pattern rules (build/r30-%.prg: ...) sit before '# Files'; a group
    # PRG such as build/r30-3.prg has no explicit rule, so without them a
    # change to its sources or vectors selected no C64 target (#235).
    for match in re.finditer(r'^([^\s:=#]*%[^\s:=#]*):(?!=)\s*(.*)\n((?:#.*\n)*)((?:\t.*\n)*)', implicit, re.M):
        patterns.append((match.group(1), set(match.group(2).replace('|', ' ').split()),
                         '\n'.join(l[1:] for l in match.group(4).splitlines())))
    rules, current = {}, None
    for line in files.splitlines():
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
    out = Rules((target, (prereqs, '\n'.join(recipe))) for target, (prereqs, recipe) in rules.items())
    out.patterns = patterns
    return out


class Rules(dict):
    """{target: (prerequisites, recipe)}, with the pattern rules as .patterns
    (callers such as ci_run.py iterate the targets)."""
    patterns = ()


def rule_for(node, rules):
    """The node's explicit rule, merged with the first pattern rule it matches."""
    prereqs, recipe = rules.get(node, ((), ''))
    if recipe:
        return prereqs, recipe
    for pattern, pre, rec in getattr(rules, 'patterns', ()):
        head, _, tail = pattern.partition('%')
        if node.startswith(head) and node.endswith(tail) and len(node) >= len(head) + len(tail):
            stem = node[len(head):len(node) - len(tail)]
            return set(prereqs) | {p.replace('%', stem) for p in pre}, rec
    return prereqs, recipe


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
        prereqs, recipe = rule_for(node, rules)
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


SPLIT_CALL = re.compile(r'\$\(call\s+CI_SPLIT_RULE\s*,\s*([A-Za-z0-9_.-]+)\s*,')
HOST_SPLIT_CALL = re.compile(r'\$\(call\s+CI_HOST_SPLIT_RULE\s*,\s*([A-Za-z0-9_.-]+)\s*,')


def mk_targets(path, rules=()):
    """Targets a .mk file defines: its literal rules, plus the c64-<m>-<n>
    and vice-<m>-<n> parts its CI_SPLIT_RULE calls and the <target>-<n> parts
    its CI_HOST_SPLIT_RULE calls make with $(eval)."""
    targets = set()
    try:
        text = (ROOT / path).read_text()
    except OSError:
        return targets
    for line in text.splitlines():
        match = re.match(r'^([A-Za-z0-9_./-]+)\s*:(?!=)', line)
        if match:
            targets.add(match.group(1))
    for milestone in SPLIT_CALL.findall(text):
        part = re.compile(r'^(c64|vice)-' + re.escape(milestone) + r'-[0-9]+$')
        targets |= {t for t in rules if part.match(t)}
    for target in HOST_SPLIT_CALL.findall(text):
        part = re.compile('^' + re.escape(target) + r'-[0-9]+$')
        targets |= {t for t in rules if part.match(t)}
    return targets


def changed_files(base):
    out = subprocess.check_output(['git', 'diff', '--name-only', f'{base}...HEAD', '--', '.'],
                                  cwd=ROOT, text=True)
    prefix = 'c64-port/'
    return [line[len(prefix):] if line.startswith(prefix) else line for line in out.splitlines()]


C_COMMENT = re.compile(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\\n])*"|\'(?:\\.|[^\'\\\n])*\'', re.S)
LINE_SENSITIVE = re.compile(r'\b(__LINE__|assert)\b')


def code_of(path, text):
    """The file's code without comments, blank lines or indentation, or None
    when this file kind is not compared (or its line numbers matter)."""
    if path.endswith('.py'):
        try:
            return ast.dump(ast.parse(text))
        except SyntaxError:
            return None
    if not path.endswith(('.c', '.h', '.S', '.s')):
        return None
    # Only C-style comments are stripped (an assembly ; comment counts as
    # code). A string continued over a line keeps its indentation, so such a
    # file is always compared as changed.
    literals = []
    code = C_COMMENT.sub(lambda m: (literals.append(m.group(0)), m.group(0))[1]
                         if m.group(0)[0] in '"\'' else ' ', text)
    if LINE_SENSITIVE.search(code) or any('\n' in lit for lit in literals):
        return None
    return '\n'.join(line.strip() for line in code.splitlines() if line.strip())


def show(rev, path):
    try:
        return subprocess.check_output(['git', 'show', f'{rev}:c64-port/{path}'], cwd=ROOT,
                                       text=True, stderr=subprocess.DEVNULL)
    except subprocess.CalledProcessError:
        return None


def code_changed(base, files):
    """Drop files whose code is the same at the merge base and HEAD."""
    mb = subprocess.check_output(['git', 'merge-base', base, 'HEAD'], cwd=ROOT, text=True).strip()
    kept = []
    for f in files:
        old, new = show(mb, f), show('HEAD', f)
        if old is not None and new is not None:
            a, b = code_of(f, old), code_of(f, new)
            if a is not None and a == b:
                continue
        kept.append(f)
    return kept


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
    if args.base:
        relevant = code_changed(args.base, relevant)
    result = {'files': len(files), 'full': any(f in SHARED for f in relevant),
              'test': [], 'sanitize': [], 'c64': [], 'vice': []}
    if relevant and not result['full']:
        rules = database()
        names = set(relevant) | {pathlib.PurePosixPath(f).name for f in relevant if '/' in f}
        defined = set()
        for f in relevant:
            if f.endswith('.mk'):
                defined |= mk_targets(f, rules)
        scripts = {}
        for group in ('test', 'sanitize', 'c64'):
            roots = sorted(t for t in rules.get(group, (set(), ''))[0] if t != 'all')
            if group == 'c64':
                roots = sorted({t for t in rules if t.startswith('c64-')} | set(roots))
            for target in roots:
                words = closure(target, rules, set(), scripts)
                if target in defined or words & names:
                    result[group].append(target)
            for target in sorted(rules.get(EXTRA[group], (set(), ''))[0]) if group in EXTRA else ():
                watch = 'ci-watch-' + target
                words = closure(watch if watch in rules else target, rules, set(), scripts)
                if (target in defined or words & names) and target not in result[group]:
                    result[group].append(target)
        for target in sorted(defined):
            for group in ('test', 'sanitize', 'c64'):
                if target.startswith(group + '-') and target not in result[group]:
                    result[group].append(target)
        for target in result['c64']:
            vice = 'vice-' + target[len('c64-'):]
            if vice in rules and vice not in result['vice']:
                result['vice'].append(vice)
        for group in ('test', 'sanitize'):
            core = group + '-core'
            if core in rules and core not in result[group] and \
                    closure(core, rules, set(), scripts) & names:
                result[group].insert(0, core)
        selected = sum(len(result[k]) for k in ('test', 'sanitize', 'c64'))
        result['full'] = selected > args.limit
        # Over the limit only because of host targets: the c64 job still runs
        # just its own list (a full c64 run is four runners of VICE).
        result['c64_full'] = len(result['c64']) > args.limit
    json.dump(result, sys.stdout, indent=1)
    print()
    return 0


if __name__ == '__main__':
    sys.exit(main())
