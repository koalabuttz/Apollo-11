#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""How much of the program is in readable C: the files listed in
readable/COVERAGE against every .agc file of both listings (Luminary099/
and Comanche055/, 175 files), weighted by source lines as
c64-port/tools/port_coverage.py weighs the faithful port.

  readable_coverage.py          print the measure
  readable_coverage.py --check  also fail if COVERAGE names a missing file
                                or directory, a directory the Makefile does
                                not build (so no gate runs it), or a file
                                README.md's table of subsystems does not
                                name
"""
import sys
from pathlib import Path

READABLE = Path(__file__).resolve().parents[1]
ROOT = READABLE.parent
TREES = ('Luminary099', 'Comanche055')


def main(argv):
    files = {str(p.relative_to(ROOT)): len(p.read_bytes().splitlines())
             for tree in TREES for p in sorted((ROOT / tree).glob('*.agc'))}
    done, bad = {}, 0
    makefile = (READABLE / 'Makefile').read_text()
    readme = (READABLE / 'README.md').read_text()
    for line in (READABLE / 'COVERAGE').read_text().splitlines():
        line = line.split('#')[0].split()
        if not line:
            continue
        path, where = line[0], line[1]
        if path not in files or not (READABLE / where).is_dir():
            print(f'FAIL: COVERAGE names {path} ({where}), which does not exist')
            bad = 1
        elif where.rstrip('/') + '/' not in makefile:
            print(f'FAIL: COVERAGE names {where} for {path}, which the Makefile does not build')
            bad = 1
        elif Path(path).name not in readme:
            print(f'FAIL: COVERAGE names {path}, which README.md does not')
            bad = 1
        done[path] = files.get(path, 0)
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
