#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Which lines of the readable sources no test and no scenario runs.

The gates say that what runs is right. This says what runs. Every target of
`make all` is built again with clang's source coverage (LINE_COVERAGE, into
build/linecov) and run, then the x86 flight program is built the same way
and flies gate 2's scenarios. Each program that ran left its counts; for
each one llvm-cov gives the count of every line of src/ compiled into it,
and a line is run if any program ran it.

The lines left are grouped into stretches (unrun lines of one function with
no run line between them), and each stretch must stand in
tests/line_coverage.txt with the reason it does not run. The check fails on
a stretch that is not there, on one that is there and now runs, and on one
with no reason. A stretch is known by its file, its function and its text
(and, where a function has two with one text, which of them it is), not by
its line numbers, so an edit elsewhere in the file does not move it.

  line_coverage.py             build, run, measure, and hold to the ledger
  line_coverage.py --measure   measure the counts of the last run again
  line_coverage.py --write     write the ledger from the measure, keeping
                               each reason whose stretch is still there
  line_coverage.py --list F    print file F (as src/...) with its counts
  line_coverage.py --self-check  the check itself on made-up measures and
                               ledgers, with no build (make all runs it)

`make line-coverage`, from readable/. Needs clang with its profile runtime
(libclang-rt-<n>-dev), llvm-profdata, llvm-cov, and the port's toolchain
for the scenarios (c64-port/tools/setup_cloud.sh). About half an hour.

A macro's lines are not counted where the macro is defined: they run where
it is used, and that line is counted. Nor is a line with no code on it.

What it cannot see: a line is run if it ran once, in any state; and a line
with two branches on it is run if either was.
"""
import collections
import hashlib
import os
import re
import shutil
import subprocess
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

READABLE = Path(__file__).resolve().parents[1]
SRC = READABLE / 'src'
BUILD = READABLE / 'build/linecov'
PROFILES = BUILD / 'profiles'
LEDGER = READABLE / 'tests/line_coverage.txt'
JOBS = 4           # of make all
SHOWN = 6          # lines of a stretch the ledger prints
NO_REASON = 'TODO'
PROFDATA = COV = None   # tools() finds them
ENTRY = re.compile(r'  (\S+), lines? (\d+)(?:-(\d+))? \((\d+) unrun\) ([0-9a-f]{8})$')
CLASS = re.compile(r'\[([a-z0-9-]+)\] (.+)$')


def tools():
    """Sets PROFDATA and COV to this clang's own llvm-profdata and
    llvm-cov, and gives the compiler's version; or exits saying what is
    missing."""
    global PROFDATA, COV
    if not shutil.which('clang'):
        sys.exit('line_coverage: no clang')

    def clang(*arguments):
        return subprocess.run(['clang', *arguments], capture_output=True, text=True, check=True).stdout.strip()
    if not Path(clang('-print-file-name=libclang_rt.profile-x86_64.a')).is_file():
        sys.exit('line_coverage: clang has no profile runtime (install libclang-rt-<version>-dev)')
    major = clang('-dumpversion').split('.')[0]
    found = []
    for tool in ('llvm-profdata', 'llvm-cov'):
        path = next((p for p in (clang(f'-print-prog-name={tool}'), f'{tool}-{major}') if shutil.which(p)), None)
        if not path:
            sys.exit(f'line_coverage: no {tool} (install llvm-{major})')
        found.append(path)
    PROFDATA, COV = found
    return clang('--version').split('\n')[0]


def fly(jobs):
    """Every target of make all, then gate 2's scenarios, each program
    writing its counts under PROFILES. A test that fails fails this."""
    PROFILES.mkdir(parents=True, exist_ok=True)
    for old in PROFILES.glob('*.profraw'):
        old.unlink()
    # %m: one file for each program (its counts added up over its runs);
    # four of them, so runs at once do not wait on one file.
    environment = {**os.environ, 'LINE_COVERAGE': '1', 'LLVM_PROFILE_FILE': str(PROFILES / '%4m.profraw')}
    # Started by make line-coverage, the makes below would be a second level
    # down, and the test scripts' own makes a third, where make prints its
    # directory lines into the lists those scripts read. So these start at
    # the top, as if typed.
    for name in ('CC', 'MAKEFLAGS', 'MFLAGS', 'MAKELEVEL'):
        environment.pop(name, None)
    for command in (['make', '-k', f'-j{jobs}', 'all'], ['make', f'-j{os.cpu_count() or 1}', 'x86-scenarios']):
        print('line_coverage:', ' '.join(command), flush=True)
        if subprocess.run(command, cwd=READABLE, env=environment, stdout=subprocess.DEVNULL).returncode:
            sys.exit(f'line_coverage: {" ".join(command)} failed with LINE_COVERAGE=1')


def programs():
    """The linked programs of the coverage build."""
    found = []
    for path in sorted(BUILD.rglob('*')):
        if path.is_file() and not path.suffix and os.access(path, os.X_OK):
            with path.open('rb') as f:
                if f.read(4) == b'\x7fELF':
                    found.append(path)
    return found


def build_id(program):
    """The program's build id, by which its counts are found; None for a
    program with no readable source compiled for coverage in it (the
    port's oracles that a script links)."""
    told = subprocess.run(['readelf', '-n', '-S', '-W', str(program)], capture_output=True, text=True,
                          check=True).stdout
    found = re.search(r'Build ID: ([0-9a-f]+)', told)
    return found.group(1) if found and '__llvm_covmap' in told else None


def profile_ids(profile):
    shown = subprocess.run([PROFDATA, 'show', '--binary-ids', str(profile)], capture_output=True, text=True,
                           check=True).stdout
    return shown.split('Binary IDs:')[1].split()


def counts(program, profiles):
    """One program's lines of src/: {file: {line: count}}, and its
    functions, {file: {name: first line}}. A function name that stands in
    two files of one program would have the two's counts added together:
    that is refused."""
    data = BUILD / 'profiles' / (program.name + '.profdata')
    subprocess.run([PROFDATA, 'merge', '-o', str(data), *map(str, profiles)], check=True)
    export = subprocess.run([COV, 'export', '-format=lcov', f'-instr-profile={data}', str(program)],
                            capture_output=True, text=True, check=True)
    data.unlink()
    if export.stderr.strip():
        sys.exit(f'line_coverage: llvm-cov on {program.name}: {export.stderr.strip()}')
    lines, functions, where = {}, {}, {}
    file, prefix = None, str(SRC) + os.sep
    for record in export.stdout.split('\n'):
        if record.startswith('SF:'):
            file = record[3:][len(prefix):] if record[3:].startswith(prefix) else None
        elif file and record.startswith('DA:'):
            line, count = record[3:].split(',')[:2]
            lines.setdefault(file, {})[int(line)] = int(count)
        elif file and record.startswith('FN:'):
            line, name = record[3:].split(',', 1)
            if where.setdefault(name, file) != file:
                sys.exit(f'line_coverage: {program.name}: {name} is a function of {where[name]} and of {file}; '
                         'their counts cannot be told apart')
            functions.setdefault(file, {})[name.split(':')[-1]] = int(line)
    return lines, functions


def no_code_lines(file):
    """The line numbers of file that hold no code to run: a preprocessor
    directive (with the lines it continues on), or nothing but space and
    comment. clang counts a macro's lines where it is defined as well as
    where it is used, and the blank and comment lines of a function a
    program never calls."""
    text = (SRC / file).read_text()
    bare = re.sub(r'/\*.*?\*/', lambda comment: re.sub(r'[^\n]', ' ', comment.group()), text, flags=re.S)
    out, inside = set(), False
    for number, (line, without) in enumerate(zip(text.split('\n'), bare.split('\n')), 1):
        if inside or line.lstrip().startswith('#'):
            out.add(number)
            inside = line.endswith('\\')
        elif not without.strip():
            out.add(number)
    return out


def measure():
    """Every program's counts, united: a line is run if any program ran
    it. Also the programs that left no counts."""
    profiles = sorted(PROFILES.glob('*.profraw'))
    if not profiles:
        sys.exit(f'line_coverage: no counts under {PROFILES.relative_to(READABLE)} (run without --measure)')
    by_id = collections.defaultdict(list)
    with ThreadPoolExecutor(os.cpu_count() or 1) as pool:
        for profile, ids in zip(profiles, pool.map(profile_ids, profiles)):
            for i in ids:
                by_id[i].append(profile)
        found = programs()
        ran = [(p, by_id.get(i)) for p, i in zip(found, pool.map(build_id, found)) if i]
        lines, functions = collections.defaultdict(dict), collections.defaultdict(dict)
        results = pool.map(lambda pr: counts(*pr), [pr for pr in ran if pr[1]])
        for program_lines, program_functions in results:
            for file, counted in program_lines.items():
                for line, count in counted.items():
                    lines[file][line] = max(lines[file].get(line, 0), count)
            for file, named in program_functions.items():
                functions[file].update(named)
    for file in list(lines):
        for line in no_code_lines(file) & set(lines[file]):
            del lines[file][line]
        if not lines[file]:
            del lines[file]
    return lines, functions, [p.name for p, profiles in ran if not profiles]


def stretches(lines, functions):
    """The unrun lines, grouped: [(file, function, first, last, unrun
    lines, digest, text)]. A stretch is the unrun lines of one function
    with no run line between them; its text is the source from its first
    line to its last."""
    out = []
    for file in sorted(lines):
        source = (SRC / file).read_text().split('\n')
        starts = sorted((line, name) for name, line in functions[file].items())

        def function(line):
            inside = [name for start, name in starts if start <= line]
            return inside[-1] if inside else '(no function)'
        runs, run = [], []
        for line in sorted(lines[file]):
            unrun = lines[file][line] == 0
            if unrun and run and function(run[0]) == function(line):
                run.append(line)
                continue
            if run:
                runs.append(run)
            run = [line] if unrun else []
        if run:
            runs.append(run)
        for run in runs:
            text = [source[n - 1].strip() for n in range(run[0], run[-1] + 1)]
            digest = hashlib.sha1('\n'.join(text).encode()).hexdigest()[:8]
            out.append((file, function(run[0]), run[0], run[-1], len(run), digest, text))
    return out


def keys(named):
    """A key for each (file, function, digest), in order: the three, and
    which stretch of that function with that text it is, counted from 0."""
    seen = collections.Counter()
    out = []
    for name in named:
        out.append((*name, seen[name]))
        seen[name] += 1
    return out


def key_of(found):
    """Each measured stretch's key."""
    return keys([(s[0], s[1], s[5]) for s in found])


def read_ledger(path=LEDGER):
    """The ledger's classes of reason, {name: text}, and its stretches,
    {key: reason} (keys(), in the ledger's order)."""
    classes, reasons, seen = {}, {}, collections.Counter()
    if not path.is_file():
        return classes, reasons
    file = key = None
    for line in path.read_text().split('\n'):
        entry = ENTRY.match(line)
        named = CLASS.match(line)
        if line.startswith('src/'):
            file = line[4:].strip()
        elif entry and file:
            name = (file, entry.group(1), entry.group(5))
            key = (*name, seen[name])
            seen[name] += 1
            reasons[key] = ''
        elif line.startswith('    why: ') and key:
            reasons[key] = line[9:].strip()
        elif line.startswith('         ') and key and reasons.get(key):
            reasons[key] += ' ' + line.strip()
        elif named and not file:
            classes[named.group(1)] = named.group(2)
        elif line.startswith('    ') and not file and classes and not line.startswith('    |'):
            last = list(classes)[-1]
            classes[last] += ' ' + line.strip()
    return classes, reasons


def wrap(text, first, rest, width=78):
    out, line = [], first
    for word in text.split():
        if len(line) + len(word) + 1 > width and line.strip() and line != first:
            out.append(line.rstrip())
            line = rest
        line += word + ' '
    return out + [line.rstrip()]


def by_reason(found, reasons):
    """How much stands under each class of reason: 'name: N lines in M
    stretches', and the same for the stretches with a reason of their own."""
    lines, count = collections.Counter(), collections.Counter()
    for s, key in zip(found, key_of(found)):
        reason = reasons.get(key, '') or NO_REASON
        name = reason[1:reason.index(']')] if reason.startswith('[') and ']' in reason else 'a reason of its own'
        lines[name] += s[4]
        count[name] += 1
    return [f'{name}: {lines[name]} lines in {count[name]} stretches' for name in sorted(lines)]


def write_ledger(path, version, lines, found, never, classes, reasons):
    total = sum(len(counted) for counted in lines.values())
    unrun = sum(s[4] for s in found)
    out = ['# The lines of readable/src that no test and no scenario runs, and why.',
           '# tests/line_coverage.py --write writes this file and keeps each reason;',
           '# `make line-coverage` measures again and holds the tree to it.',
           '#',
           f'# Measured with {version}.',
           f'# {len(lines)} files have lines that run; {total} such lines; {unrun} of them unrun',
           f'# ({100 * unrun / total:.2f}%), in {len(found)} stretches of {len({s[0] for s in found})} files.',
           '# Built and never run: ' + (', '.join(never) or 'no program') + '.',
           '#',
           '# By reason:',
           *[f'#   {line}' for line in by_reason(found, reasons)],
           '#',
           '# A reason in brackets is one of these:',
           '']
    for name in sorted(classes):
        out += wrap(classes[name], f'[{name}] ', '    ')
    out.append('')
    file = None
    for s, key in zip(found, key_of(found)):
        if s[0] != file:
            file = s[0]
            out += ['', f'src/{file}']
        span = f'line {s[2]}' if s[2] == s[3] else f'lines {s[2]}-{s[3]}'
        out.append(f'  {s[1]}, {span} ({s[4]} unrun) {s[5]}')
        text = [t for t in s[6] if t]
        out += [f'    | {t}' for t in text[:SHOWN]]
        if len(text) > SHOWN:
            out.append(f'    | ... ({len(text) - SHOWN} more)')
        reason = reasons.get(key, '') or NO_REASON
        out += wrap(reason, '    why: ', '         ')
    path.write_text('\n'.join(out) + '\n')


def judge(found, classes, reasons):
    """What is wrong between the measured stretches and the ledger: the
    stretches not in it, its keys that are not unrun stretches now, and its
    keys with no reason (none, NO_REASON, or a class its head does not
    define)."""
    measured = dict(zip(key_of(found), found))
    new = [s for key, s in measured.items() if key not in reasons]
    gone = [key for key in reasons if key not in measured]
    unexplained = [key for key, reason in reasons.items() if key in measured and
                   (not reason or reason == NO_REASON or
                    (reason.startswith('[') and reason[1:reason.index(']')] not in classes))]
    return new, gone, unexplained


def self_check():
    """The check on made-up measures, with no build: a ledger written and
    read back holds the measure it was written from, and each way a ledger
    can be wrong is reported as that and nothing else. Two of the stretches
    are of one function and have one text."""
    def stretch(file, function, line, text):
        digest = hashlib.sha1('\n'.join(text).encode()).hexdigest()[:8]
        return (file, function, line, line + len(text) - 1, len(text), digest, text)
    found = [stretch('a/a.c', 'first', 10, ['return 0; /* never: no such word */']),
             stretch('a/a.c', 'twice', 20, ['leave();']),
             stretch('a/a.c', 'twice', 30, ['leave();']),
             stretch('b/b.c', 'long_one', 5, [f'x += {n};' for n in range(SHOWN + 3)])]
    classes = {'never': 'The comment says the outcome never comes, a text long enough that the ledger has '
                        'to wrap it onto a second line of its head.',
               'refusal': 'How a binding refuses.'}
    whys = ['[never]', '[refusal]', '[refusal]', 'A reason of its own, long enough that the ledger has to wrap '
            'it onto a second line under the stretch it is the reason of.']
    reasons = dict(zip(key_of(found), whys))
    lines = {'a/a.c': {10: 0, 20: 0, 30: 0, 40: 1}, 'b/b.c': {n: 0 for n in range(5, 5 + SHOWN + 3)}}
    failures = []

    def expect(name, measured, edit, want):
        with tempfile.TemporaryDirectory() as scratch:
            ledger = Path(scratch) / 'ledger.txt'
            write_ledger(ledger, 'no compiler', lines, found, [], classes, reasons)
            ledger.write_text(edit(ledger.read_text()))
            got = tuple(len(part) for part in judge(measured, *read_ledger(ledger)))
        if got != want:
            failures.append(f'{name}: {got} (not in the ledger, not unrun now, no reason), expected {want}')

    def same(text):
        return text
    expect('a ledger read back', found, same, (0, 0, 0))
    if not failures:
        with tempfile.TemporaryDirectory() as scratch:
            ledger = Path(scratch) / 'ledger.txt'
            write_ledger(ledger, 'no compiler', lines, found, [], classes, reasons)
            if read_ledger(ledger) != (classes, reasons):
                failures.append('a ledger read back is not the classes and reasons it was written from')
    expect('a new unrun stretch', found + [stretch('b/b.c', 'other', 90, ['y = 1;'])], same, (1, 0, 0))
    expect('a stretch that now runs', found[1:], same, (0, 1, 0))
    expect('the first of two with one text now runs', found[:1] + found[2:], same, (0, 1, 0))
    expect('a third with the same text', found[:3] + [stretch('a/a.c', 'twice', 35, ['leave();'])] + found[3:],
           same, (1, 0, 0))
    expect('a text that changed', [stretch('a/a.c', 'first', 10, ['return 1;'])] + found[1:], same, (1, 1, 0))
    expect('no reason', found, lambda text: text.replace('why: [never]', 'why: ' + NO_REASON), (0, 0, 1))
    expect('a class the head does not define', found, lambda text: text.replace('why: [never]', 'why: [nonesuch]'),
           (0, 0, 1))
    for failure in failures:
        print(f'FAIL line coverage self-check: {failure}')
    print(f"{'FAIL' if failures else 'ok  '} line coverage self-check: 9 made-up measures and ledgers, "
          f'{len(failures)} judged wrongly')
    return 1 if failures else 0


def listing(lines, file):
    counted = lines.get(file[4:] if file.startswith('src/') else file)
    if counted is None:
        sys.exit(f'line_coverage: no line of {file} is in a program that ran')
    for number, text in enumerate((READABLE / 'src' / (file[4:] if file.startswith('src/') else file))
                                  .read_text().split('\n'), 1):
        count = counted.get(number)
        mark = '        ' if count is None else '   #####' if count == 0 else f'{min(count, 99999999):8d}'
        print(f'{mark} {number:5d} {text}')


def main():
    args = sys.argv[1:]
    if '--self-check' in args:
        return self_check()
    version = tools()
    jobs = int(args[args.index('--jobs') + 1]) if '--jobs' in args else JOBS
    if not {'--measure', '--write', '--list'} & set(args) or '--fly' in args:
        fly(jobs)
    lines, functions, never = measure()
    if '--list' in args:
        listing(lines, args[args.index('--list') + 1])
        return 0
    found = stretches(lines, functions)
    classes, reasons = read_ledger()
    total = sum(len(counted) for counted in lines.values())
    unrun = sum(s[4] for s in found)
    print(f'line coverage: {len(lines)} files, {total} lines, {unrun} unrun ({100 * unrun / total:.2f}%) in '
          f'{len(found)} stretches of {len({s[0] for s in found})} files; built and never run: '
          f'{", ".join(never) or "no program"}')
    if '--write' in args:
        write_ledger(LEDGER, version, lines, found, never, classes, reasons)
        print(f'wrote {LEDGER.relative_to(READABLE)}')
        return 0
    for line in by_reason(found, reasons):
        print(f'  {line}')
    new, gone, unexplained = judge(found, classes, reasons)
    for s in new[:40]:
        print(f'FAIL src/{s[0]}: {s[1]}, lines {s[2]}-{s[3]} ({s[4]} unrun) is not run and not in the ledger')
    for key in gone[:40]:
        print(f'FAIL src/{key[0]}: {key[1]} {key[2]} is in the ledger and is not an unrun stretch now')
    for key in unexplained[:40]:
        print(f'FAIL src/{key[0]}: {key[1]} {key[2]} has no reason')
    bad = len(new) + len(gone) + len(unexplained)
    print(f"{'FAIL' if bad else 'ok  '} line coverage against {LEDGER.relative_to(READABLE)}: {len(new)} unrun "
          f'stretches not in it, {len(gone)} of its stretches not unrun now, {len(unexplained)} without a reason')
    if bad:
        # What --write would make of this measure, for a run whose tree
        # is not at hand (CI keeps it).
        write_ledger(BUILD / LEDGER.name, version, lines, found, never, classes, reasons)
        print(f'the ledger as measured is {(BUILD / LEDGER.name).relative_to(READABLE)}')
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())
