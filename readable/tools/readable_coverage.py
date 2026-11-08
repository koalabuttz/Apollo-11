#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""How much of the program is in readable C: the files listed in
readable/COVERAGE against every .agc file of both listings (Luminary099/
and Comanche055/, 175 files), weighted by source lines as
c64-port/tools/port_coverage.py weighs the faithful port.

  readable_coverage.py          print the measure
  readable_coverage.py --check  also fail if COVERAGE names a missing file
                                or directory, a directory the Makefile does
                                not build (so no gate runs it), a file
                                README.md's table of subsystems does not
                                name, a binding that hands keys to the
                                faithful tables without naming the partial
                                file it writes (or naming a whole one), or
                                a file whose no-code mark does not hold

A listing file of comments only (no line of code: every line blank or a
comment) has no routine to write; COVERAGE lists it with the directory
whose header carries its words, marked no-code, and it counts by its
lines like any other. The check holds the mark to the listing both ways.
"""
import re
import sys
from pathlib import Path

READABLE = Path(__file__).resolve().parents[1]
ROOT = READABLE.parent
TREES = ('Luminary099', 'Comanche055')
MISSIONS = {'lm': 'Luminary099', 'cm': 'Comanche055'}
# A binding that runs the faithful port's routine at a key no part has
# written yet names a faithful table, renamed for the readable build
# (tests/x86_readable.py): faithful_<table>. Any faithful_ name in the
# code (not in a comment), whatever it is called and however it is used,
# counts. (Pinball's faithful trampoline, faithful_lanec_pin_run_<m>, is
# not a table and not counted: it runs only a step the readable code does
# not model, src/binding/pinball_routines.c.)
FALLBACK = re.compile(r'\bfaithful_\w+')
TRAMPOLINE = 'faithful_lanec_pin_run_'
COMMENT = re.compile(r'/\*.*?\*/|//[^\n]*', re.S)
# Such a binding names the listing file it writes part by part, in a
# comment: "Partial file: Comanche055/AUTOMATIC_MANEUVERS.agc". That file
# cannot be whole; a binding with a fallback and no such line fails.
PARTIAL = re.compile(r'Partial file: ((?:Luminary099|Comanche055)/[\w.-]+\.agc)')


def fallbacks(done):
    """The bindings that hand keys to the faithful tables and name no
    partial file, or name one COVERAGE counts whole: (binding, message)."""
    found = []
    for binding in sorted((READABLE / 'src/binding').glob('*.c')):
        text = binding.read_text()
        names = sorted({n for n in FALLBACK.findall(COMMENT.sub(' ', text))
                        if not n.startswith(TRAMPOLINE)})
        if not names:
            continue
        partial = PARTIAL.findall(text)
        if not partial:
            found.append((binding.relative_to(READABLE),
                          f'hands keys to the faithful tables ({", ".join(names)}) '
                          'but names no partial file'))
        for path in partial:
            if path in done:
                found.append((binding.relative_to(READABLE),
                              f'still hands keys to the faithful tables ({", ".join(names)}), '
                              f'but COVERAGE names {path} whole'))
    return found


def code_line(path):
    """The first line of code in a listing file (not blank, not a
    comment), as (number, text), or None."""
    for number, line in enumerate((ROOT / path).read_text().splitlines(), 1):
        if line.strip() and not line.lstrip().startswith('#'):
            return number, line.strip()
    return None


def main(argv):
    files = {str(p.relative_to(ROOT)): len(p.read_bytes().splitlines())
             for tree in TREES for p in sorted((ROOT / tree).glob('*.agc'))}
    done, where_of, bad = {}, {}, 0
    makefile = (READABLE / 'Makefile').read_text()
    readme = (READABLE / 'README.md').read_text()
    for line in (READABLE / 'COVERAGE').read_text().splitlines():
        line = line.split('#')[0].split()
        if not line:
            continue
        path, where = line[0], line[1]
        no_code = line[2:] == ['no-code']
        if line[2:] and not no_code:
            print(f'FAIL: COVERAGE line for {path} ends in {" ".join(line[2:])}, not no-code')
            bad = 1
        if path not in files or not (READABLE / where).is_dir():
            print(f'FAIL: COVERAGE names {path} ({where}), which does not exist')
            bad = 1
        elif where.rstrip('/') + '/' not in makefile:
            print(f'FAIL: COVERAGE names {where} for {path}, which the Makefile does not build')
            bad = 1
        elif Path(path).name not in readme:
            print(f'FAIL: COVERAGE names {path}, which README.md does not')
            bad = 1
        elif no_code and code_line(path):
            number, text = code_line(path)
            print(f'FAIL: COVERAGE marks {path} no-code, but its line {number} is code: {text}')
            bad = 1
        elif not no_code and not code_line(path):
            print(f'FAIL: {path} holds no code: COVERAGE must mark it no-code')
            bad = 1
        done[path] = files.get(path, 0)
        where_of[path] = where
    for binding, message in fallbacks(where_of):
        print(f'FAIL: {binding} {message}')
        bad = 1
    total = sum(files.values())
    print(f'Readable C: {len(done)} of {len(files)} files, '
          f'{sum(done.values()):,} of {total:,} source lines '
          f'({100.0 * sum(done.values()) / total:.1f}%)')
    if done:
        name = max({Path(p).name for p in done},
                   key=lambda n: sum(v for p, v in done.items() if Path(p).name == n))
        lines = sum(v for p, v in done.items() if Path(p).name == name)
        print(f'  the largest share: {name}, {100.0 * lines / sum(done.values()):.0f}% '
              'of the readable lines')
    for tree in TREES:
        mine = {p: n for p, n in files.items() if p.startswith(tree + '/')}
        got = sum(n for p, n in done.items() if p.startswith(tree + '/'))
        print(f'  {tree}: {sum(p in done for p in mine)} of {len(mine)} files, '
              f'{100.0 * got / sum(mine.values()):.1f}% of its lines')
    return bad if '--check' in argv else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
