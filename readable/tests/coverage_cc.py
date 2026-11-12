#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""The compiler of the line measure's build: clang, but for five things.

clang keeps a function's counts under its name, and llvm-cov shows a
program's counts by name. Where a program holds two functions of one name,
their counts are added together when the two are alike in shape, and only
one of the two has its lines shown at all. The measure
(tests/line_coverage.py) must read each count as one function's, so this
build (the Makefile's CC when LINE_COVERAGE is set, and the linker of the
scripts that link readable objects themselves, tests/coverage_build.py)
gives no program two measured functions of one name:

1. Only the sources under readable/src are compiled with counters. The
   tests' own sources and the faithful port's are not measured, and some of
   them hold functions named as the readable ones are (a test's own
   dispatch that hands each mission's cases to that mission's routines).

2. A static function's name is its file's and its own (programs.c:tc_to),
   and clang takes the file's base name for it: two files of one base name
   with a static function of one name make one name. Here each source is
   compiled with its path from the repository as its file's name
   (readable/src/p32_p35/programs.c:tc_to).

3. A test that links two builds of one source holds every function of it
   twice: both missions' builds, each made into one object with its names
   kept local, as most of the tests of both missions do. Such a program
   is linked once for each build, with the other build's counters and line
   maps cut out of it (the counters' sections renamed, so that clang's
   runtime does not find them, and the maps taken out): OUT+lm and OUT+cm,
   named after the objects. OUT itself is then a script that runs each of
   them in turn, so that the test runs as it does in the ordinary build,
   twice: the first run is the test, with its output and its result; the
   second must end well too, and its output, which is the same, is dropped.

4. Each program writes its counts to files of its own name under
   build/linecov/profiles (clang's runtime otherwise names the files by the
   sizes of what a program measures, and two tests of one set of sources
   would write into one file). The object that holds the name is kept
   beside the program, as PROGRAM.counts.o.

5. What the measure counts is every line clang compiled, in a program that
   ran or not. So each compile of a source under readable/src leaves that
   object's lines and functions under build/linecov/maps, with the
   source's name at the head (a source that holds only tables leaves its
   name and nothing else).

  coverage_cc.py ARGUMENTS   (clang's own)

Not part of the ordinary build, which never runs this.
"""
import functools
import os
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

READABLE = Path(__file__).resolve().parents[1]
REPOSITORY = READABLE.parent
SRC = str(READABLE / 'src') + os.sep
BUILD = READABLE / 'build/linecov'
PROFILES = BUILD / 'profiles'
MAPS_KEPT = BUILD / 'maps'
COUNTING = ('-fprofile-instr-generate', '-fcoverage-mapping')
# What clang's runtime finds a build's counters by, and llvm-cov its line
# maps. (The maps go altogether: the linker keeps one map for each name,
# the first it meets, whatever its section is called.)
COUNTERS = ('__llvm_prf_cnts', '__llvm_prf_data', '__llvm_prf_names', '__llvm_prf_vnds', '__llvm_prf_bits')
MAPS = ('__llvm_covfun', '__llvm_covmap')
SCRIPT = '''#!/bin/sh
# Made by tests/coverage_cc.py: this test holds {count} builds of one source.
# Each program below has one build's counters, and the others' cut out.
{runs}
'''


def clang(*arguments):
    return subprocess.run(['clang', *arguments], capture_output=True, text=True, check=True).stdout.strip()


@functools.lru_cache(None)
def tool(name):
    """clang's own llvm-profdata or llvm-cov, as tests/line_coverage.py finds it."""
    major = clang('-dumpversion').split('.')[0]
    path = next((p for p in (clang(f'-print-prog-name={name}'), f'{name}-{major}') if shutil.which(p)), None)
    if not path:
        sys.exit(f'coverage_cc: no {name} (install llvm-{major})')
    return path


def no_counts():
    """A file of no counts, which llvm-cov needs to read an object's maps."""
    path = BUILD / 'no-counts.profdata'
    if not path.is_file():
        BUILD.mkdir(parents=True, exist_ok=True)
        with tempfile.TemporaryDirectory(dir=BUILD) as scratch:
            (Path(scratch) / 'none.proftext').write_text('')
            made = Path(scratch) / path.name
            subprocess.run([tool('llvm-profdata'), 'merge', '-o', str(made), str(Path(scratch) / 'none.proftext')],
                           check=True)
            os.replace(made, path)
    return path


def mapped(obj):
    """The records of an object's line maps that are of readable/src: each
    file (SF:), its functions (FN:) and its lines (DA:), with no counts."""
    told = subprocess.run(['readelf', '-S', '-W', obj], capture_output=True, text=True).stdout
    if '__llvm_covfun' not in told:
        return []
    export = subprocess.run([tool('llvm-cov'), 'export', '-format=lcov', f'-instr-profile={no_counts()}', obj],
                            capture_output=True, text=True, check=True).stdout
    out, inside = [], False
    for record in export.split('\n'):
        if record.startswith('SF:'):
            inside = record[3:].startswith(SRC)
        if inside and record.startswith(('SF:', 'FN:', 'DA:')):
            out.append(record)
    return out


def keep_map(source, obj, name):
    """Leave what obj, compiled from source, holds to run: under MAPS_KEPT,
    by name."""
    MAPS_KEPT.mkdir(parents=True, exist_ok=True)
    kept = MAPS_KEPT / (re.sub(r'[^A-Za-z0-9_.+-]', '_', name) + '.lcov')
    scratch = Path(f'{kept}.{os.getpid()}')
    scratch.write_text('\n'.join([f'SOURCE:{os.path.relpath(source, SRC)}', *mapped(obj)]) + '\n')
    os.replace(scratch, kept)


def held(obj):
    """The names of the functions of readable/src that an object holds
    with counters (kept beside the object, for the next link of it)."""
    kept = Path(obj + '.held')
    if kept.is_file() and kept.stat().st_mtime >= Path(obj).stat().st_mtime:
        return set(kept.read_text().split('\n')) - {''}
    names = {record.split(',', 1)[1] for record in mapped(obj) if record.startswith('FN:')}
    scratch = Path(f'{kept}.{os.getpid()}')
    scratch.write_text('\n'.join(sorted(names)))
    os.replace(scratch, kept)
    return names


def builds(objects):
    """The objects that hold a function another of them holds too, in
    groups: each group's objects hold no function twice among them."""
    names = {o: held(o) for o in objects}
    twice = [o for o in objects if any(names[o] & names[other] for other in objects if other != o)]
    groups = []
    for o in twice:
        for group in groups:
            if not any(names[o] & names[other] for other in group):
                group.append(o)
                break
        else:
            groups.append([o])
    return groups


def tags(groups):
    """A name for each group: its mission, where its objects' names say
    one, or its number."""
    out = []
    for number, group in enumerate(groups, 1):
        said = {m for o in group for m in re.findall(r'(?<![a-z0-9])(lm|cm)(?![a-z0-9])', Path(o).name)}
        out.append(said.pop() if len(said) == 1 else str(number))
    return out if len(set(out)) == len(out) else [str(number) for number in range(1, len(groups) + 1)]


def compile_one(flags, source, out=None, name=None):
    """One source: with counters and its path as its name if it is under
    readable/src, without counters if not. (out: the object, where flags
    does not say; name: what to keep its map under, where the object is
    not kept.)"""
    path = os.path.realpath(source)
    measured = path.startswith(SRC) and all(f in flags for f in COUNTING)
    if measured:
        flags = [*flags, '-Xclang', '-main-file-name', '-Xclang', os.path.relpath(path, REPOSITORY)]
    else:
        flags = [f for f in flags if f not in COUNTING]
    status = subprocess.run(['clang', *flags, *(['-c', source, '-o', out] if out else [])]).returncode
    if measured and not status and '-E' not in flags and '-S' not in flags and '-fsyntax-only' not in flags:
        obj = out or (flags[flags.index('-o') + 1] if '-o' in flags else Path(source).stem + '.o')
        obj = os.path.realpath(obj)
        # (An object outside the build is known by its source too: two
        # compiles to one scratch path must not take each other's place.)
        keep_map(path, obj, name or (os.path.relpath(obj, BUILD) if obj.startswith(str(BUILD) + os.sep)
                                     else f'{obj}.of.{os.path.relpath(path, SRC)}'))
    return status


def link_one(arguments, out, program, scratch):
    """One program, which writes its counts under its own name."""
    program = Path(program).resolve()
    inside = program.is_relative_to(BUILD)
    name = '__'.join(program.relative_to(BUILD).parts) if inside else program.name
    named = Path(scratch) / f'{program.name}.counts.c'
    named.write_text(f'const char __llvm_profile_filename[] = "{PROFILES / name}.%4m.profraw";\n')
    subprocess.run(['clang', '-c', str(named), '-o', f'{program}.counts.o'], check=True)
    return subprocess.run(['clang', *(str(program) if a == out else a for a in arguments),
                           f'{program}.counts.o']).returncode


def link(arguments, out, groups, scratch):
    """OUT+tag for each group, with every other group's counters and maps
    cut out, and OUT, the script that runs them."""
    runs, cut = [], {}
    for number, o in enumerate(o for group in groups for o in group):
        cut[o] = str(Path(scratch) / f'cut{number}-{Path(o).name}')
        subprocess.run(['objcopy', *(f'--rename-section={s}=__cut{s[6:]}' for s in COUNTERS),
                        *(f'--remove-section={s}' for s in MAPS), o, cut[o]], check=True)
    for group, tag in zip(groups, tags(groups)):
        status = link_one([cut[a] if a in cut and a not in group else a for a in arguments], out, f'{out}+{tag}',
                          scratch)
        if status:
            return status
        runs.append(f'"$0+{tag}" "$@"' + (' > /dev/null' if runs else '') + ' || exit $?')
    Path(out).write_text(SCRIPT.format(count=len(groups), runs='\n'.join(runs)))
    Path(out).chmod(0o755)
    return 0


def main(arguments):
    sources = [a for a in arguments if a.endswith('.c')]
    if '-c' in arguments or '-o' not in arguments:
        if len(sources) > 1 and any(os.path.realpath(s).startswith(SRC) for s in sources):
            sys.exit('coverage_cc: more sources than one in a command that only compiles')
        return compile_one(arguments, sources[0]) if sources else subprocess.run(['clang', *arguments]).returncode
    out = arguments[arguments.index('-o') + 1]
    Path(out).parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(dir=Path(out).parent) as scratch:
        # The sources of a command that links too, each on its own.
        flags = [a for a in arguments
                 if not a.endswith(('.c', '.o')) and a not in ('-o', out) and not a.startswith(('-Wl,', '-l'))]
        for number, source in enumerate(sources):
            compiled = str(Path(scratch) / f'{number}-{Path(source).stem}.o')
            status = compile_one(flags, source, compiled, f'{os.path.relpath(os.path.realpath(out), BUILD)}.{number}')
            if status:
                return status
            arguments = [compiled if a == source else a for a in arguments]
        groups = builds([a for a in arguments if a.endswith('.o') and a != out])
        if len(groups) < 2:
            return link_one(arguments, out, out, scratch)
        return link(arguments, out, groups, scratch)


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
