#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Which lines of the readable sources no test and no scenario runs.

The gates say that what runs is right. This says what runs. Every target of
`make all` is built again with clang's source coverage (LINE_COVERAGE, into
build/linecov) and run, then the x86 flight program is built the same way
and flies gate 2's scenarios. Each program that ran left its counts; for
each one llvm-cov gives the count of every line of src/ compiled into it,
and a line is run if any program ran it. What is counted is every line the
build compiled, in a program that ran or not: each compile leaves its
object's lines (tests/coverage_cc.py, under build/linecov/maps), and a
line of a function that no program holds is a line not run.

The lines left are grouped into stretches (unrun lines of one function with
no run line between them), and each stretch must stand in
tests/line_coverage.txt with the reason it does not run. The check fails on
a stretch that is not there, on one that is there and now runs, and on one
whose reason does not stand: none, one of no class, or one of a class that
does not fit the stretch (fits(), below: a [refusal] outside the bindings,
a [not-modelled] with no not_modelled() at its lines, a [never] with no
comment there that says never, a [never] or an [impossible] that does not
say why). A stretch is known by its file, its function and its text (and,
where a function has two with one text, which of them it is), not by its
line numbers, so an edit elsewhere in the file does not move it. The line
numbers and the counts in the ledger are those of the measure it was last
written from, for the reader; they are not compared.

Every file under src/ must be accounted for too. A .c file has lines that
are counted, or the ledger names it among the files that are not, as one
the build compiles that holds no line to run ([no-code]: tables, names and
interpretive words) or as one no target compiles ([not-built], with why).
A header or .inc file is counted where a source that includes it is
compiled, so it must be read by a source the build compiles, directly or
through another header, or be named [not-built] too. The check fails on a
file that is not counted and not named, on one named that is counted now,
and on one named as the wrong kind.

A line can run and still be compared with nothing: one that refuses
(not_modelled() first on it). A case or a scenario brought the routine
there, the binding refused the state, and the test held only that the
refusal left the state as it found it. Such a line is not unrun, so it is
in no stretch; the ledger lists them at its end, by function with how many,
and the check fails when a function's count is not the measure's
(refusals_run()). So a [not-modelled] stretch that a new case comes to
moves to that list, and does not just go.

What is measured is C. An interpretive program is a table of words to the
compiler, with no line to run: the measure does not see which of its words
the interpreter ran, in a [no-code] source or in a source that has C lines
of its own beside its tables.

  line_coverage.py             build, run, measure, and hold to the ledger
  line_coverage.py --measure   measure the counts of the last run again
  line_coverage.py --write     write the ledger from the measure, keeping
                               each reason whose stretch is still there
  line_coverage.py --list F    print file F (as src/...) with its counts
  line_coverage.py --self-check  the check itself on made-up measures and
                               ledgers, with no build (make all runs it)
  line_coverage.py --jobs N    make all with N jobs at once (4)
  line_coverage.py --guard-check  only the measure's guard, on a small
                               program built for the purpose (needs clang;
                               the first step of a run)

`make line-coverage`, from readable/. Needs clang with its profile runtime
(libclang-rt-<n>-dev), llvm-profdata, llvm-cov, and the port's toolchain
for the scenarios (c64-port/tools/setup_cloud.sh). About half an hour.

A macro's lines are not counted where the macro is defined: they run where
it is used, and that line is counted. Nor is a line with no code on it.

What it cannot see: a line is run if it ran once, in any state; and a line
with two branches on it is run if either was. Nor can the check tell a
reason that is wrong from one that is right: it holds each reason to its
class's form, and a person reads the argument.
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
MAPS = BUILD / 'maps'
LEDGER = READABLE / 'tests/line_coverage.txt'
JOBS = 4           # of make all
SHOWN = 6          # lines of a stretch the ledger prints
NO_REASON = 'TODO'
PROFDATA = COV = None   # tools() finds them
ENTRY = re.compile(r'  (\S+), lines? (\d+)(?:-(\d+))? \((\d+) unrun\) ([0-9a-f]{8})$')
CLASS = re.compile(r'\[([a-z0-9-]+)\] (.+)$')
REASON = re.compile(r'\[([a-z0-9-]+)\](?: (.+))?$', re.S)
SOURCE = re.compile(r'  - src/(\S+): (.*)$')
SOURCES_HEAD = 'Files of src/ that are not counted:'
# A line that refuses: not_modelled() first on it, after any case labels.
REFUSES = re.compile(r'\s*(?:(?:case \w+|default):\s*)*not_modelled\(\);')
REFUSED_HEAD = 'Lines that ran and refuse (not_modelled() first on the line), by function:'
REFUSED = re.compile(r'  \* src/(\S+): (\S+) \((\d+)\)$')
# The classes a source with no line counted is named under, and whether the
# build compiles a source of that class.
SOURCE_CLASSES = {'no-code': True, 'not-built': False}


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
    if guard_check():
        sys.exit('line_coverage: the guard does not hold; nothing built')
    PROFILES.mkdir(parents=True, exist_ok=True)
    for old in PROFILES.glob('*.profraw'):
        old.unlink()
    # Each program writes its counts to files of its own name there
    # (tests/coverage_cc.py links the name in; its counts are added up over
    # its runs, in four files, so that runs at once do not wait on one).
    environment = {**os.environ, 'LINE_COVERAGE': '1'}
    environment.pop('LLVM_PROFILE_FILE', None)
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


TOY = {
    # Two files of one base name, each with a static function of one name.
    'src/one/part.c': 'static int helper(int x) { return x + 1; }\nint one(int x) { return helper(x); }\n',
    'src/two/part.c': 'static int helper(int x) { return x + 2; }\nint two(int x) { return helper(x); }\n',
    # One source built once for each mission, each build's names kept local.
    'src/both.c': '#if LM\nstatic int mission(void) { return 1; }\n#else\nstatic int mission(void) { return 2; }\n'
                  '#endif\nint shared(int x)\n{\n    return x + mission();\n}\n'
                  'int ENTRY(void) { return shared(1); }\n',
    # The test itself, which is not measured.
    'main.c': 'int one(int), two(int), lm_entry(void), cm_entry(void);\n'
              'int main(void) { return one(1) + two(1) + lm_entry() + cm_entry() == 10 ? 0 : 1; }\n',
}
TOY_OBJECTS = (('one.o', 'src/one/part.c', []), ('two.o', 'src/two/part.c', []),
               ('both-lm.o', 'src/both.c', ['-DLM=1', '-DENTRY=lm_entry']),
               ('both-cm.o', 'src/both.c', ['-DLM=0', '-DENTRY=cm_entry']), ('main.o', 'main.c', []))


def guard_check():
    """The measure's guard and the build that keeps it quiet, tried on a
    small program that has both things the guard is for: two files of one
    base name with a static function of one name, and one source built for
    both missions and linked into one test. Built plainly by clang, the
    program holds those names twice and the guard must refuse it. Built by
    tests/coverage_cc.py it is one program for each mission, none holding
    a name twice, and each function's count is its own. Returns the number
    of things that are not so."""
    import importlib.util
    spec = importlib.util.spec_from_file_location('toy_coverage_cc', READABLE / 'tests/coverage_cc.py')
    cc = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(cc)
    scratch = BUILD / 'guard-check'
    shutil.rmtree(scratch, ignore_errors=True)
    for name, text in TOY.items():
        (scratch / name).parent.mkdir(parents=True, exist_ok=True)
        (scratch / name).write_text(text)
    # (The compiler takes this directory for readable/src, and keeps what
    # it leaves apart from the real build's.)
    cc.SRC, cc.BUILD = str(scratch / 'src') + os.sep, scratch
    cc.PROFILES, cc.MAPS_KEPT = scratch / 'profiles', scratch / 'maps'
    cc.PROFILES.mkdir()
    counting = ['-std=c99', '-O2', '-fprofile-instr-generate', '-fcoverage-mapping']
    failures = []

    def build(kind, compile_, link):
        objects = []
        for obj, source, defines in TOY_OBJECTS:
            made = scratch / f'{kind}-{obj}'
            compile_([*counting, *defines, '-c', str(scratch / source), '-o', f'{made}.all'])
            entry = [d.split('=')[1] for d in defines if d.startswith('-DENTRY=')]
            subprocess.run(['objcopy', *(f'--keep-global-symbol={e}' for e in entry), f'{made}.all', str(made)],
                           check=True)
            objects.append(str(made))
        link(['-fprofile-instr-generate', *objects, '-o', str(scratch / kind)])
        return scratch / kind

    def held(program, raw):
        """A program's lines of the toy sources and the names it holds twice."""
        data = scratch / (program.name + '.profdata')
        subprocess.run([PROFDATA, 'merge', '-o', str(data), str(raw)], check=True)
        export = subprocess.run([COV, 'export', '-format=lcov', f'-instr-profile={data}', str(program)],
                                capture_output=True, text=True, check=True).stdout
        counted, file = {}, None
        for record in export.split('\n'):
            if record.startswith('SF:'):
                file = record[3:][len(cc.SRC):] if record[3:].startswith(cc.SRC) else None
            elif file and record.startswith('DA:'):
                counted[(file, int(record[3:].split(',')[0]))] = int(record[3:].split(',')[1])
        return counted, held_twice(raw)

    # Plainly: the names are held twice, and the guard says so.
    def plainly(arguments):
        subprocess.run(['clang', *arguments], check=True)
    plain = build('plain', plainly, plainly)
    raw = scratch / 'plain.profraw'
    subprocess.run([str(plain)], env={**os.environ, 'LLVM_PROFILE_FILE': str(raw)}, check=True)
    _, twice_held = held(plain, raw)
    for name in ('shared', 'part.c:helper'):
        if name not in twice_held:
            failures.append(f'built plainly, the program is not seen to hold {name} twice')
    if not refused('plain', twice_held, {'shared', 'part.c:helper'}):
        failures.append('the guard does not refuse a program that holds a measured name twice')

    # By the measure's compiler: a program for each mission, none of them
    # holding a name twice, and the line of shared() run once in each.
    def by_wrapper(arguments):
        if cc.main([str(a) for a in arguments]):
            failures.append('tests/coverage_cc.py did not build the program')
    built = build('measured', by_wrapper, by_wrapper)
    environment = {key: value for key, value in os.environ.items() if key != 'LLVM_PROFILE_FILE'}
    if not failures and subprocess.run([str(built)], env=environment).returncode:
        failures.append('the program built by tests/coverage_cc.py does not run')
    parts = sorted(scratch.glob('measured+*'))
    if [part.name for part in parts if not part.name.endswith('.o')] != ['measured+cm', 'measured+lm']:
        failures.append('tests/coverage_cc.py did not link a program for each mission')
    for part in [part for part in parts if not part.name.endswith('.o')] if not failures else []:
        raws = sorted(cc.PROFILES.glob(part.name + '.*.profraw'))
        if len(raws) != 1:
            failures.append(f'{part.name} left {len(raws)} count files')
            continue
        counted, twice_held = held(part, raws[0])
        if twice_held:
            failures.append(f'{part.name} holds two functions named {sorted(twice_held)[0]}')
        want = {('one/part.c', 1): 1, ('one/part.c', 2): 1, ('two/part.c', 1): 1, ('two/part.c', 2): 1,
                ('both.c', 2 if part.name.endswith('lm') else 4): 1,
                **{('both.c', line): 1 for line in (7, 8, 9, 10)}}
        if counted != want:
            failures.append(f'{part.name} counts {sorted(counted.items())}, not each of its lines once')
    sources, lines, _ = compiled([kept.read_text() for kept in sorted(cc.MAPS_KEPT.glob('*.lcov'))])
    if sources != {'one/part.c', 'two/part.c', 'both.c'}:
        failures.append(f'the compiles left the lines of {sorted(sources)}')
    if not failures:
        shutil.rmtree(scratch)
    for failure in failures:
        print(f'FAIL line coverage guard check: {failure}')
    print(f"{'FAIL' if failures else 'ok  '} line coverage guard check: a program that holds two functions of one "
          'name is refused when built plainly, and is two programs with a count for each function when built '
          f'by tests/coverage_cc.py; {len(failures)} not so')
    return len(failures)


def programs():
    """The linked programs of the coverage build."""
    found = []
    for path in sorted(BUILD.rglob('*')):
        if BUILD / 'guard-check' in path.parents:
            continue
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


def twice(shown):
    """The names listed more than once in what llvm-profdata shows of a
    count file's functions."""
    held = collections.Counter(re.findall(r'^  (\S+):$', shown, re.M))
    return {name for name, times in held.items() if times > 1}


def held_twice(profile):
    """The names a program holds more than one function under, from one of
    its count files, which lists every function it holds."""
    return twice(subprocess.run([PROFDATA, 'show', '--all-functions', str(profile)], capture_output=True, text=True,
                                check=True).stdout)


def refused(program, held, measured):
    """Why a program's counts cannot be used: it holds two functions under
    a name the measure counts (held: the names it holds twice, measured:
    the names of every function of src/). Nothing if it holds none."""
    both = sorted(held & measured)
    if not both:
        return ''
    return (f'{program} holds two functions named {both[0]}'
            + (f' (and of {len(both) - 1} more names)' if len(both) > 1 else '')
            + '; their counts cannot be told apart (tests/coverage_cc.py)')


def counts(program, profiles):
    """One program's lines of src/: {file: {line: count}}, its functions,
    {file: {(first line, name)}}, their names as clang keeps their counts,
    and the names the program holds two functions under. A function name
    that stands in two files of one program would have the two's counts
    added together: that is refused."""
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
            functions.setdefault(file, set()).add((int(line), name.split(':')[-1]))
    return lines, functions, set(where), held_twice(profiles[0])


def compiled(maps=None):
    """What the build compiled of src/, from what each compile left under
    MAPS (tests/coverage_cc.py; maps: their texts): the sources, every
    line with code, {file: {line: 0}}, and the functions, {file: {(first
    line, name)}}."""
    sources, lines, functions = set(), {}, {}
    prefix = str(SRC) + os.sep
    if maps is None:
        maps = [kept.read_text() for kept in sorted(MAPS.glob('*.lcov'))]
    for text in maps:
        file = None
        for record in text.split('\n'):
            if record.startswith('SOURCE:'):
                sources.add(record[7:])
            elif record.startswith('SF:'):
                file = record[3:][len(prefix):] if record[3:].startswith(prefix) else None
            elif file and record.startswith('DA:'):
                lines.setdefault(file, {})[int(record[3:].split(',')[0])] = 0
            elif file and record.startswith('FN:'):
                line, name = record[3:].split(',', 1)
                functions.setdefault(file, set()).add((int(line), name.split(':')[-1]))
    return sources, lines, functions


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


def united(results, built_lines, built_functions):
    """Every program's counts, united with what the build compiled: a line
    is run if any program ran it, and a line the build compiled into no
    program that ran is not run (results: each program's lines and
    functions, as counts() gives them)."""
    lines, functions = collections.defaultdict(dict), collections.defaultdict(set)
    for program_lines, program_functions in results:
        for file, counted in program_lines.items():
            for line, count in counted.items():
                lines[file][line] = max(lines[file].get(line, 0), count)
        for file, named in program_functions.items():
            functions[file] |= named
    for file, counted in built_lines.items():
        for line in counted:
            lines[file].setdefault(line, 0)
        functions[file] |= built_functions.get(file, set())
    return lines, functions


def measure():
    """Every program's counts, united (united()). Also the programs that
    left no counts, and the sources of src/ that the build compiled."""
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
        counted_programs = [pr for pr in ran if pr[1]]
        results = list(pool.map(lambda pr: counts(*pr), counted_programs))
    # A program that holds two functions under one name has their counts
    # added together where the two are alike in shape, and shows the lines
    # of one of them only (both missions' builds of one source in one test;
    # two files' static functions of one name). tests/coverage_cc.py builds
    # every program so that none does; one that does is refused here.
    measured = set().union(*(names for _, _, names, _ in results))
    for (program, _), (_, _, _, held) in zip(counted_programs, results):
        why = refused(program.name, held, measured)
        if why:
            sys.exit(f'line_coverage: {why}')
    sources, built_lines, built_functions = compiled()
    if not sources:
        sys.exit(f'line_coverage: no compile left its lines under {MAPS.relative_to(READABLE)} '
                 '(run without --measure)')
    lines, functions = united([result[:2] for result in results], built_lines, built_functions)
    for file in list(lines):
        for line in no_code_lines(file) & set(lines[file]):
            del lines[file][line]
        if not lines[file]:
            del lines[file]
    return lines, functions, [p.name for p, profiles in ran if not profiles], sources


def function_at(starts):
    """The function a line is of: the one that starts last before it."""
    starts = sorted(starts)

    def function(line):
        inside = [name for start, name in starts if start <= line]
        return inside[-1] if inside else '(no function)'
    return function


def refusals_run(lines, functions, text_of=lambda file: (SRC / file).read_text()):
    """The lines that ran and refuse, {(file, function): how many}: a line
    with a count that has not_modelled() first on it. A case or a scenario
    brought the routine there, and its binding refused the state: the line
    is run, and nothing was compared at it but that the refusal left the
    state as it found it. (A not_modelled() that follows a test on its own
    line is not seen: the line runs whenever the test does.)"""
    out = collections.Counter()
    for file in sorted(lines):
        source = text_of(file).split('\n')
        function = function_at(functions[file])
        for line in sorted(lines[file]):
            if lines[file][line] and line <= len(source) and REFUSES.match(source[line - 1]):
                out[file, function(line)] += 1
    return dict(out)


def stretches(lines, functions, text_of=lambda file: (SRC / file).read_text()):
    """The unrun lines, grouped: [(file, function, first, last, unrun
    lines, digest, text)]. A stretch is the unrun lines of one function
    with no run line between them; its text is the source from its first
    line to its last. (A file may hold two functions of one name, one for
    each mission: a line is of the function that starts last before it.)"""
    out = []
    for file in sorted(lines):
        source = text_of(file).split('\n')
        function = function_at(functions[file])
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
    """The ledger's classes of reason, {name: text}, its stretches, {key:
    reason} (keys(), in the ledger's order), the sources it names as having
    no line counted, {file: reason}, and the lines it lists as run and
    refusing, {(file, function): how many}."""
    classes, reasons, sources, refusing, seen = {}, {}, {}, {}, collections.Counter()
    if not path.is_file():
        return classes, reasons, sources, refusing
    file = key = listing = tail = None
    for line in path.read_text().split('\n'):
        entry = ENTRY.match(line)
        named = CLASS.match(line)
        source = SOURCE.match(line)
        refuser = REFUSED.match(line)
        if line == REFUSED_HEAD:
            tail, key = True, None
        elif tail:
            if refuser:
                refusing[refuser.group(1), refuser.group(2)] = int(refuser.group(3))
        elif line == SOURCES_HEAD:
            listing = True
        elif listing and source and not file:
            listing = source.group(1)
            sources[listing] = source.group(2).strip()
        elif listing and listing is not True and line.startswith('      ') and not file:
            sources[listing] += ' ' + line.strip()
        elif line.startswith('src/'):
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
        elif named and not file and not listing:
            classes[named.group(1)] = named.group(2)
        elif line.startswith('    ') and not file and not listing and classes and not line.startswith('    |'):
            last = list(classes)[-1]
            classes[last] += ' ' + line.strip()
    return classes, reasons, sources, refusing


def wrap(text, first, rest, width=78):
    out, line = [], first
    for word in text.split():
        if len(line) + len(word) + 1 > width and line.strip() and line != first:
            out.append(line.rstrip())
            line = rest
        line += word + ' '
    return out + [line.rstrip()]


def class_of(reason):
    """A reason's class and what it says after it: (name, why), or (None,
    reason) for one that starts with no class."""
    told = REASON.match(reason or '')
    return (told.group(1), (told.group(2) or '').strip()) if told else (None, reason or '')


def fits(name, why, file, text):
    """What is wrong with a reason of class name, saying why, for a stretch
    of file with that text; nothing if it fits. Each class of the ledger's
    head has its rule here."""
    if name == 'refusal':
        return '' if file.startswith('binding/') else 'is [refusal] and not in a binding (src/binding)'
    if name == 'not-modelled':
        return '' if 'not_modelled' in text else 'is [not-modelled] with no not_modelled() at its lines'
    if name == 'never' and not re.search(r'\bnever\b', text, re.I):
        return 'is [never] with no comment at its lines that says never'
    if name in ('never', 'impossible'):
        return '' if why else f'is [{name}] and does not say why'
    return '' if name == 'open' else f'has a class this check has no rule for ([{name}])'


def source_fits(name, why, built):
    """The same for a source the ledger names as having no line counted
    (built: whether the build compiled it)."""
    if name not in SOURCE_CLASSES:
        return f'is named under [{name}], which is no class of a source' if name else 'has no class'
    if SOURCE_CLASSES[name] != built:
        return f'is named [{name}] and the build ' + ('compiles it' if built else 'does not compile it')
    return '' if built or why else 'is [not-built] and does not say why'


def includes_of(file, text=None):
    """The files of src/ that a file of src/ includes by name: in quotes,
    or by a macro the file defines as a name in quotes (each such
    definition: a source for both missions defines one for each)."""
    text = (SRC / file).read_text() if text is None else text
    names = re.findall(r'^[ \t]*#[ \t]*include[ \t]*"([^"]+)"', text, re.M)
    for macro in re.findall(r'^[ \t]*#[ \t]*include[ \t]+([A-Za-z_]\w*)[ \t]*$', text, re.M):
        names += re.findall(rf'^[ \t]*#[ \t]*define[ \t]+{macro}[ \t]+"([^"]+)"', text, re.M)
    out = set()
    for name in names:
        path = Path(os.path.normpath(SRC / file / '..' / name))
        if path.is_file() and path.is_relative_to(SRC):
            out.add(str(path.relative_to(SRC)))
    return out


def read_by(compiled_sources, includes=includes_of):
    """The files of src/ the build reads: the sources it compiles and
    every file one of them includes, directly or through another."""
    read, left = set(), list(compiled_sources)
    while left:
        file = left.pop()
        if file not in read:
            read.add(file)
            left += includes(file)
    return read


def files_of_src():
    return sorted(str(path.relative_to(SRC)) for path in SRC.rglob('*') if path.suffix in ('.c', '.h', '.inc'))


def without_lines(lines, read, named, files=None):
    """The files of src/ that the measure does not count, {file: reason}:
    a .c file with no line counted, and a header (or .inc) that no source
    the build compiles reads. (A header the build reads is tied through
    the sources that read it: a function added to it would be counted.)
    The reason is the ledger's for each file it names; for one it does
    not, [no-code] if the build reads it, and none if not. (read: the
    files the build reads, read_by(); files: those of src/.)"""
    out = {}
    for file in files_of_src() if files is None else files:
        if file not in lines and (file.endswith('.c') or file not in read):
            out[file] = named.get(file) or ('[no-code]' if file in read else NO_REASON)
    return out


def interpretive(lines):
    """How much interpretive code src/ holds, which this measure does not
    count: the programs (each a table of words, a struct
    interpretive_code), the files they are in, and how many of those
    files have lines of C that are counted."""
    programs, files = 0, set()
    for file in files_of_src():
        held = len(re.findall(r'^(?:static )?const struct interpretive_code \w+ = \{',
                              (SRC / file).read_text(), re.M))
        if held:
            programs += held
            files.add(file)
    return programs, len(files), len(files & set(lines))


def by_reason(found, reasons):
    """How much stands under each class of reason: 'name: N lines in M
    stretches', and the same for the stretches with a reason of their own."""
    lines, count = collections.Counter(), collections.Counter()
    for s, key in zip(found, key_of(found)):
        name = class_of(reasons.get(key, ''))[0] or 'no class'
        lines[name] += s[4]
        count[name] += 1
    return [f'{name}: {lines[name]} lines in {count[name]} stretches' for name in sorted(lines)]


def write_ledger(path, version, lines, found, never, classes, reasons, sources, refusing=None):
    """(sources: the files of src/ the measure does not count, {file: reason};
    refusing: the lines that ran and refuse, refusals_run().)"""
    refusing = refusing or {}
    total = sum(len(counted) for counted in lines.values())
    unrun = sum(s[4] for s in found)
    sources_with = sum(1 for file in lines if file.endswith('.c'))
    sources_without = sum(1 for file in sources if file.endswith('.c'))
    programs, program_files, with_lines = interpretive(lines)
    out = ['# The lines of readable/src that no test and no scenario runs, and why.',
           '# tests/line_coverage.py --write writes this file and keeps each reason;',
           '# `make line-coverage` measures again and holds the tree to it.',
           '#',
           f'# Measured with {version}.',
           f'# {len(lines)} files hold lines of C to run ({sources_with} of the {sources_with + sources_without} '
           '.c files under src/,',
           f'# and {len(lines) - sources_with} headers); {total} such lines; {unrun} of them unrun',
           f'# ({100 * unrun / total:.2f}%), in {len(found)} stretches of {len({s[0] for s in found})} files.',
           '# Built and never run: ' + (', '.join(never) or 'no program') + '.',
           '#',
           '# These are lines of C only. Interpretive code is tables of words, which',
           f'# hold no line to run: {programs} interpretive programs in {program_files} files of src/',
           f'# ({with_lines} of them files with C lines that are counted, the rest among the',
           '# files not counted, below). Which of their words the interpreter ran,',
           '# this measure does not see.',
           '#',
           '# By reason:',
           *[f'#   {line}' for line in by_reason(found, reasons)],
           '#',
           f'# Run, and compared with nothing: {sum(refusing.values())} lines of {len(refusing)} functions ran',
           '# and refuse (not_modelled() first on the line). A case or a scenario',
           '# brought the routine there and its binding refused the state: the tests',
           '# hold only that a refusal leaves the state as it found it. They are',
           '# listed at the end, by function.',
           '#',
           '# A reason in brackets is one of these:',
           '']
    for name in sorted(classes):
        out += wrap(classes[name], f'[{name}] ', '    ')
    out += ['', SOURCES_HEAD]
    for file in sorted(sources):
        out += wrap(sources[file] or NO_REASON, f'  - src/{file}: ', '      ')
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
    out += ['', REFUSED_HEAD]
    out += [f'  * src/{file}: {function} ({count})' for (file, function), count in sorted(refusing.items())]
    path.write_text('\n'.join(out) + '\n')


def refusals_differ(measured, listed):
    """What is wrong between the lines that ran and refuse and the ledger's
    list of them, a line each."""
    out = []
    for file, function in sorted(set(measured) | set(listed)):
        ran, told = measured.get((file, function), 0), listed.get((file, function), 0)
        if ran != told:
            out.append(f'src/{file}: {function} has {ran} lines that ran and refuse, and the ledger lists {told}')
    return out


def judge(found, classes, reasons, sources=None, named=None, compiled_sources=()):
    """What is wrong between the measure and the ledger: the measured
    stretches not in it; its keys that are not unrun stretches now; its
    keys whose reason does not stand, each with what is wrong, [(key,
    what)] (no reason, one of no class or of a class its head does not
    define, or one that does not fit the stretch: fits()); and what is
    wrong with the sources it names as having no line counted, a line each
    (sources: those the measure found, without_lines(); named: the
    ledger's; compiled_sources: the files the build reads, read_by())."""
    measured = dict(zip(key_of(found), found))
    new = [s for key, s in measured.items() if key not in reasons]
    gone = [key for key in reasons if key not in measured]
    unexplained = []
    for key, reason in reasons.items():
        if key not in measured:
            continue
        name, why = class_of(reason)
        if not reason or reason == NO_REASON:
            what = 'has no reason'
        elif name is None:
            what = 'has a reason of no class'
        elif name not in classes:
            what = f'has a class the head does not define ([{name}])'
        else:
            what = fits(name, why, key[0], '\n'.join(measured[key][6]))
        if what:
            unexplained.append((key, what))
    files = []
    sources, named = sources or {}, named or {}
    for file in sorted(set(sources) | set(named)):
        if file not in named:
            files.append(f'src/{file} has no line counted and the ledger does not name it'
                         + ('' if file in compiled_sources else
                            ' (no target compiles it)' if file.endswith('.c') else
                            ' (no source the build compiles reads it)'))
        elif file not in sources:
            files.append(f'src/{file} is named as not counted and '
                         + ('is no file' if not (SRC / file).is_file() else
                            'has lines now' if file.endswith('.c') else 'the build reads it now'))
        else:
            name, why = class_of(named[file])
            what = source_fits(name, why, file in compiled_sources)
            if what:
                files.append(f'src/{file} {what}')
    return new, gone, unexplained, files


def self_check():
    """The check on made-up measures, with no build: a ledger written and
    read back holds the measure it was written from, and each way a ledger
    can be wrong is reported as that and nothing else. Two of the stretches
    are of one function and have one text."""
    def stretch(file, function, line, text):
        digest = hashlib.sha1('\n'.join(text).encode()).hexdigest()[:8]
        return (file, function, line, line + len(text) - 1, len(text), digest, text)
    found = [stretch('a/a.c', 'first', 10, ['return 0; /* never: no such word */']),
             stretch('binding/a.c', 'twice', 20, ['leave();']),
             stretch('binding/a.c', 'twice', 30, ['leave();']),
             stretch('b/b.c', 'long_one', 5, [f'x += {n};' for n in range(SHOWN + 3)]),
             stretch('b/b.c', 'marked', 40, ['default: not_modelled(); return;'])]
    classes = {'never': 'The comment says the outcome never comes, a text long enough that the ledger has '
                        'to wrap it onto a second line of its head.',
               'impossible': 'The lines before rule it out.',
               'not-modelled': 'Marked so.',
               'open': 'Not looked at yet.',
               'refusal': 'How a binding refuses.',
               'no-code': 'A source of tables.',
               'not-built': 'A source no target compiles.'}
    whys = ['[never] No program assembles such a word.', '[refusal]', '[refusal]',
            '[impossible] A reason long enough that the ledger has to wrap it onto a second line under the '
            'stretch it is the reason of.', '[not-modelled]']
    reasons = dict(zip(key_of(found), whys))
    lines = {'a/a.c': {10: 0, 40: 1}, 'binding/a.c': {20: 0, 30: 0},
             'b/b.c': {**{n: 0 for n in range(5, 5 + SHOWN + 3)}, 40: 0}}
    named = {'t/tables.c': '[no-code]',
             't/spare.c': '[not-built] Kept for a port that is not started, a reason long enough to wrap onto '
                          'a second line.'}
    built = {'t/tables.c'}
    refusing = {('a/a.c', 'first'): 2, ('b/b.c', 'marked'): 1}
    failures, tried = [], []

    def expect(name, measured, edit, want, sources=None, compiled_sources=None):
        tried.append(name)
        with tempfile.TemporaryDirectory() as scratch:
            ledger = Path(scratch) / 'ledger.txt'
            write_ledger(ledger, 'no compiler', lines, found, [], classes, reasons, named, refusing)
            ledger.write_text(edit(ledger.read_text()))
            read = read_ledger(ledger)
            got = tuple(len(part) for part in judge(measured, read[0], read[1], named if sources is None else sources,
                                                    read[2], built if compiled_sources is None else compiled_sources))
        if got != want:
            failures.append(f'{name}: {got} (not in the ledger, not unrun now, a reason that does not stand, '
                            f'sources), expected {want}')

    def same(text):
        return text

    def swap(old, new):
        def edit(text):
            if text.count(old) != 1:
                failures.append(f'the self-check\'s own ledger has not one "{old}"')
            return text.replace(old, new)
        return edit
    expect('a ledger read back', found, same, (0, 0, 0, 0))
    if not failures:
        with tempfile.TemporaryDirectory() as scratch:
            ledger = Path(scratch) / 'ledger.txt'
            write_ledger(ledger, 'no compiler', lines, found, [], classes, reasons, named, refusing)
            if read_ledger(ledger) != (classes, reasons, named, refusing):
                failures.append('a ledger read back is not the classes, reasons, sources and refusing lines it '
                                'was written from')
    expect('a new unrun stretch', found + [stretch('b/b.c', 'other', 90, ['y = 1;'])], same, (1, 0, 0, 0))
    expect('a stretch that now runs', found[1:], same, (0, 1, 0, 0))
    expect('the first of two with one text now runs', found[:1] + found[2:], same, (0, 1, 0, 0))
    expect('a third with the same text',
           found[:3] + [stretch('binding/a.c', 'twice', 35, ['leave();'])] + found[3:], same, (1, 0, 0, 0))
    expect('a text that changed', [stretch('a/a.c', 'first', 10, ['return 1;'])] + found[1:], same, (1, 1, 0, 0))
    # A reason that does not stand.
    never, impossible = 'why: [never] No program assembles such a word.', 'why: [impossible] A reason long'
    expect('no reason', found, swap(never, 'why: ' + NO_REASON), (0, 0, 1, 0))
    expect('a class the head does not define', found, swap(never, 'why: [nonesuch] No such word.'), (0, 0, 1, 0))
    expect('a reason of no class', found, swap(never, 'why: No program assembles such a word.'), (0, 0, 1, 0))
    expect('a class that is not closed', found, swap(never, 'why: [never No such word.'), (0, 0, 1, 0))
    expect('a source\'s class on a stretch', found, swap(never, 'why: [no-code]'), (0, 0, 1, 0))
    expect('a [never] that does not say why', found, swap(never, 'why: [never]'), (0, 0, 1, 0))
    expect('an [impossible] that does not say why', found,
           lambda text: re.sub(r'why: \[impossible\][^\n]*\n(?:         [^\n]*\n)*', 'why: [impossible]\n', text),
           (0, 0, 1, 0))
    expect('a [never] whose lines do not say never', found, swap(impossible, 'why: [never] A reason long'),
           (0, 0, 1, 0))
    expect('a [refusal] outside the bindings', found, swap(never, 'why: [refusal]'), (0, 0, 1, 0))
    expect('a [not-modelled] with no not_modelled()', found, swap(never, 'why: [not-modelled]'), (0, 0, 1, 0))
    expect('an [open] stretch', found, swap(never, 'why: [open]'), (0, 0, 0, 0))
    # The sources with no line counted.
    expect('a source with no lines that the ledger does not name', found, same, (0, 0, 0, 1),
           sources={**named, 't/new.c': ''})
    expect('a named source that has lines now', found, same, (0, 0, 0, 1), sources={'t/tables.c': ''})
    expect('a [no-code] source the build does not compile', found, same, (0, 0, 0, 1), compiled_sources=set())
    expect('a [not-built] source the build compiles', found, same, (0, 0, 0, 1),
           compiled_sources={'t/tables.c', 't/spare.c'})
    expect('a [not-built] source that does not say why', found,
           lambda text: re.sub(r'(spare\.c: \[not-built\])[^\n]*\n(?:      [^\n]*\n)*', r'\1\n', text),
           (0, 0, 0, 1))
    expect('a source with no class', found, swap('tables.c: [no-code]', 'tables.c: tables'), (0, 0, 0, 1))
    expect('a source under a stretch\'s class', found, swap('tables.c: [no-code]', 'tables.c: [never] tables'),
           (0, 0, 0, 1))
    expect('a header that no compiled source reads, and the ledger does not name', found, same, (0, 0, 0, 1),
           sources={**named, 't/spare.h': ''})

    # The parts of the measure itself, each on made-up data.
    def part(name, got, want):
        tried.append(name)
        if got != want:
            failures.append(f'{name}: {got!r}, expected {want!r}')
    # The lines that ran and refuse.
    refuser = ['void first(void)', '{', '    switch (x) {', '    case A: not_modelled(); return;',
               '    case B:', '    default: not_modelled();', '    }', '    if (y) not_modelled();',
               '    not_modelled();', '}', 'void other(void)', '{', '    not_modelled();', '}']
    part('the lines that ran and refuse',
         refusals_run({'a/a.c': {4: 3, 6: 1, 8: 9, 9: 0, 13: 0}}, {'a/a.c': {(1, 'first'), (11, 'other')}},
                      lambda file: '\n'.join(refuser)), {('a/a.c', 'first'): 2})
    with tempfile.TemporaryDirectory() as scratch:
        ledger = Path(scratch) / 'ledger.txt'
        write_ledger(ledger, 'no compiler', lines, found, [], classes, reasons, named, refusing)
        listed = read_ledger(ledger)[3]
    part('the list of them read back', listed, refusing)
    part('one that ran as listed', refusals_differ(refusing, listed), [])
    part('a refusal that ran and is not listed',
         len(refusals_differ({**refusing, ('b/b.c', 'other'): 1}, listed)), 1)
    part('a listed one that did not run', len(refusals_differ({('a/a.c', 'first'): 2}, listed)), 1)
    part('one more line of a listed function', len(refusals_differ({**refusing, ('a/a.c', 'first'): 3}, listed)), 1)
    shown = 'Counters:\n  fetch:\n    Hash: 0x1\n  store:\n    Hash: 0x2\n  fetch:\n    Hash: 0x1\n  main:\n'
    part('a count file that lists a name twice', twice(shown), {'fetch'})
    part('a count file that lists each name once', twice(shown.replace('  store', '  other', 1).replace(
        '  fetch:\n    Hash: 0x1\n  main', '  main')), set())
    part('a program that holds a measured name twice', bool(refused('p', {'fetch'}, {'fetch', 'store'})), True)
    part('a program that holds only a name of its own twice', refused('p', {'main'}, {'fetch', 'store'}), '')
    part('a program that holds no name twice', refused('p', set(), {'fetch', 'store'}), '')
    src = str(SRC) + os.sep
    maps = [f'SOURCE:a/a.c\nSF:{src}a/a.c\nFN:3,readable/src/a/a.c:helper\nFN:9,first\nDA:4,0\nDA:10,0\n'
            f'SF:/elsewhere/{"x" * len(src)}/other.h\nFN:1,theirs\nDA:1,0\nSF:{src}a/a.h\nFN:2,inlined\nDA:2,0\n',
            'SOURCE:t/tables.c\n']
    built_lines = {'a/a.c': {4: 0, 10: 0}, 'a/a.h': {2: 0}}
    built_functions = {'a/a.c': {(3, 'helper'), (9, 'first')}, 'a/a.h': {(2, 'inlined')}}
    part('what the compiles left', compiled(maps), ({'a/a.c', 't/tables.c'}, built_lines, built_functions))
    all_lines, all_functions = united([({'a/a.c': {10: 0, 11: 2}}, {'a/a.c': {(9, 'first')}}),
                                       ({'a/a.c': {10: 5, 11: 0}}, {})], built_lines, built_functions)
    part('counts united: the most any program counted, and 0 for a compiled line in no program',
         (dict(all_lines), dict(all_functions)),
         ({'a/a.c': {10: 5, 11: 2, 4: 0}, 'a/a.h': {2: 0}}, built_functions))
    # One file with a function of one name for each mission, and one between.
    pair = stretches({'m/m.c': {3: 0, 12: 0, 22: 0}}, {'m/m.c': {(1, 'same'), (10, 'other'), (20, 'same')}},
                     lambda file: '\n'.join(f'line {n};' for n in range(1, 30)))
    part('two functions of one name in one file', [(s[1], s[2]) for s in pair],
         [('same', 3), ('other', 12), ('same', 22)])
    part('what a source includes: in quotes, and through a macro of its own for each mission',
         includes_of('agc/agc.c', '#include <stdio.h>\n#include "agc.h"\n#if LUMINARY099\n'
                     '#define NAMES "lm_names.inc"\n#else\n#define NAMES "cm_names.inc"\n#endif\n'
                     '#include NAMES\n#include "no_such_file.h"\n'),
         {'agc/agc.h', 'agc/lm_names.inc', 'agc/cm_names.inc'})
    includes = {'a/a.c': {'a/a.h'}, 'a/a.h': {'agc/names.h'}, 'agc/names.h': set(), 't/tables.c': set()}
    read = read_by({'a/a.c', 't/tables.c'}, lambda file: includes[file])
    part('the files the build reads', read, {'a/a.c', 'a/a.h', 'agc/names.h', 't/tables.c'})
    part('the files that are not counted',
         without_lines({'a/a.c': {10: 0}, 'a/a.h': {2: 1}}, read, {'t/spare.c': '[not-built] Why.'},
                       ['a/a.c', 'a/a.h', 'a/spare.h', 'agc/names.h', 't/new.c', 't/spare.c', 't/tables.c']),
         {'a/spare.h': NO_REASON, 't/new.c': NO_REASON, 't/spare.c': '[not-built] Why.', 't/tables.c': '[no-code]'})
    for failure in failures:
        print(f'FAIL line coverage self-check: {failure}')
    print(f"{'FAIL' if failures else 'ok  '} line coverage self-check: {len(tried)} made-up measures, ledgers and "
          f'parts of a measure, {len(failures)} judged wrongly')
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


OPTIONS = ('--measure', '--write', '--list', '--self-check', '--fly', '--jobs', '--guard-check')


def main():
    args = [part for a in sys.argv[1:] for part in (a.split('=', 1) if a.startswith('--') else [a])]
    # (An option this does not know must not start half an hour's build.)
    unknown = [a for a in args if a.startswith('-') and a not in OPTIONS]
    if unknown:
        print(__doc__ if unknown[0] in ('-h', '--help') else f'line_coverage: no option {unknown[0]} (see --help)')
        return 0 if unknown[0] in ('-h', '--help') else 2
    if '--self-check' in args:
        return self_check()
    version = tools()
    if '--guard-check' in args:
        return 1 if guard_check() else 0
    jobs = int(args[args.index('--jobs') + 1]) if '--jobs' in args else JOBS
    if not {'--measure', '--write', '--list'} & set(args) or '--fly' in args:
        fly(jobs)
    lines, functions, never, compiled_sources = measure()
    if '--list' in args:
        listing(lines, args[args.index('--list') + 1])
        return 0
    found = stretches(lines, functions)
    refusing = refusals_run(lines, functions)
    classes, reasons, named, listed = read_ledger()
    compiled_sources = read_by(compiled_sources)
    sources = without_lines(lines, compiled_sources, named)
    total = sum(len(counted) for counted in lines.values())
    unrun = sum(s[4] for s in found)
    print(f'line coverage: {len(lines)} files, {total} lines, {unrun} unrun ({100 * unrun / total:.2f}%) in '
          f'{len(found)} stretches of {len({s[0] for s in found})} files; {sum(refusing.values())} lines that '
          f'ran and refuse; {len(sources)} files of src/ not counted; built and never run: '
          f'{", ".join(never) or "no program"}')
    if '--write' in args:
        write_ledger(LEDGER, version, lines, found, never, classes, reasons, sources, refusing)
        print(f'wrote {LEDGER.relative_to(READABLE)}')
        return 0
    for line in by_reason(found, reasons):
        print(f'  {line}')
    new, gone, unexplained, files = judge(found, classes, reasons, sources, named, compiled_sources)
    for s in new[:40]:
        print(f'FAIL src/{s[0]}: {s[1]}, lines {s[2]}-{s[3]} ({s[4]} unrun) is not run and not in the ledger')
    for key in gone[:40]:
        print(f'FAIL src/{key[0]}: {key[1]} {key[2]} is in the ledger and is not an unrun stretch now')
    for key, what in unexplained[:40]:
        print(f'FAIL src/{key[0]}: {key[1]} {key[2]} {what}')
    for line in files[:40]:
        print(f'FAIL {line}')
    refusals = refusals_differ(refusing, listed)
    for line in refusals[:40]:
        print(f'FAIL {line}')
    bad = len(new) + len(gone) + len(unexplained) + len(files) + len(refusals)
    print(f"{'FAIL' if bad else 'ok  '} line coverage against {LEDGER.relative_to(READABLE)}: {len(new)} unrun "
          f'stretches not in it, {len(gone)} of its stretches not unrun now, {len(unexplained)} with a reason '
          f'that does not stand, {len(files)} files not accounted for, {len(refusals)} functions whose lines '
          'that ran and refuse are not as listed')
    if bad:
        # What --write would make of this measure, for a run whose tree
        # is not at hand (CI keeps it).
        write_ledger(BUILD / LEDGER.name, version, lines, found, never, classes, reasons, sources, refusing)
        print(f'the ledger as measured is {(BUILD / LEDGER.name).relative_to(READABLE)}')
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())
