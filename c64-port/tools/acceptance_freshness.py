#!/usr/bin/env python3
"""Says whether each program's whole-program acceptance still describes the
C64 build it would ship.

Each program's bootable acceptance doc (docs/acceptance/<program>-
bootable_c64_application.md) lists the sha256 of the files the C64 runs:
the REU store, the flight resident and the loader. This tool hashes the same
files under build/ (run `make flight-disk` first) and reports each program
as FRESH when all three match, or STALE with the files that moved. A STALE
program's acceptance was recorded against an older build, so it needs a
re-acceptance run at the current commit before anyone quotes it for the
shipped build.

port_coverage.py --require-complete checks that acceptance exists; this
checks that it is current. --require-fresh exits 1 on any STALE program or
missing build file.
"""
import argparse
import hashlib
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
PROGRAMS = {'Luminary099': 'lm', 'Comanche055': 'cm'}
ROW = re.compile(r'^\| `([^`]+)` \| `([0-9a-f]{64})` \|$')


def payload(m):
    """The files whose bytes the C64 runs, as the bootable doc names them."""
    return [f'flight-store/{m}.reu', f'flight-disk/{m}-resident.prg', f'flight-disk/{m}-loader.prg']


def recorded(program):
    doc = ROOT / 'docs/acceptance' / f'{program}-bootable_c64_application.md'
    text = doc.read_text()
    commit = re.search(r'at commit `([0-9a-f]+)`', text)
    rows = {}
    for line in text.splitlines():
        match = ROW.match(line.strip())
        if match:
            rows[match.group(1)] = match.group(2)
    return (commit.group(1) if commit else '?'), rows


def check(program, m, build):
    commit, rows = recorded(program)
    moved, missing = [], []
    for name in payload(m):
        want = rows.get(name)
        path = build / name
        if want is None:
            missing.append(f'{name} (not in the doc)')
        elif not path.exists():
            missing.append(f'{name} (not built)')
        elif hashlib.sha256(path.read_bytes()).hexdigest() != want:
            moved.append(name)
    return commit, moved, missing


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--build', type=pathlib.Path, default=ROOT / 'build')
    parser.add_argument('--require-fresh', action='store_true')
    args = parser.parse_args()
    bad = False
    for program, m in PROGRAMS.items():
        commit, moved, missing = check(program, m, args.build)
        if missing:
            bad = True
            print(f'{program}: UNKNOWN (accepted at {commit}); missing: {", ".join(missing)}')
        elif moved:
            bad = True
            print(f'{program}: STALE (accepted at {commit}); changed since: {", ".join(moved)}')
        else:
            print(f'{program}: FRESH (accepted at {commit}; store, resident and loader match the build)')
    return 1 if bad and args.require_fresh else 0


if __name__ == '__main__':
    sys.exit(main())
