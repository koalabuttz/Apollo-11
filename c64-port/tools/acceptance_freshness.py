#!/usr/bin/env python3
"""Says whether each program's whole-program acceptance still describes the
C64 build it would ship.

Each program's bootable acceptance doc (docs/acceptance/<program>-
bootable_c64_application.md) lists the sha256 of the files the C64 runs:
the REU store, the flight resident, the loader and the disk images. This
tool hashes the same files under build/ (run `make flight-disk` first) and
reports each program as FRESH when every built one matches (the store,
resident and loader must be built), or STALE with the files that moved. It
prints the checkout's commit so a build/ older than the checkout is visible. A STALE
program's acceptance was recorded against an older build, so it needs a
re-acceptance run at the current commit before anyone quotes it for the
shipped build.

port_coverage.py --require-complete checks that acceptance exists; this
checks that it is current. --require-fresh exits 1 on any STALE program or
missing build file. port-ci runs it on every PR that touches C64 code
(job acceptance-fresh), so a PR that moves an accepted program's bytes must
carry that program's re-acceptance (CONTRIBUTING-port.md, "After
acceptance").

docs/acceptance/pending-reacceptance.json lists programs the base already
knows are stale: the PR that made them so, and the sha256 of each file that
moved at that point. A listed program reports STALE without failing only
while every moved file still has its pinned hash, so any further byte move
fails as usual. The re-acceptance PR deletes the entry; an entry can only be
added in a reviewed PR.
"""
import json
import argparse
import hashlib
import pathlib
import re
import subprocess
import sys
import time

ROOT = pathlib.Path(__file__).resolve().parent.parent
PROGRAMS = {'Luminary099': 'lm', 'Comanche055': 'cm'}
PENDING = ROOT / 'docs/acceptance/pending-reacceptance.json'
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
    """Every file the doc lists is compared when it is built; the store,
    resident and loader must be. Disk images exist only after the runs that
    make them (the 1541 sets come from --vice), so an unbuilt image is
    reported as unchecked rather than missing."""
    commit, rows = recorded(program)
    moved, missing, unchecked = [], [], []
    for name in payload(m):
        if name not in rows:
            missing.append(f'{name} (not in the doc)')
    for name, want in rows.items():
        path = build / name
        if not path.exists() and name in payload(m):
            missing.append(f'{name} (not built)')
        elif not path.exists():
            unchecked.append(name)
        elif hashlib.sha256(path.read_bytes()).hexdigest() != want:
            moved.append(name)
    return commit, moved, missing, unchecked


def head():
    try:
        out = subprocess.run(['git', 'rev-parse', '--short=12', 'HEAD'], cwd=ROOT, capture_output=True, text=True)
        dirty = subprocess.run(['git', 'status', '--porcelain', '--untracked-files=no'], cwd=ROOT,
                               capture_output=True, text=True).stdout.strip()
        return out.stdout.strip() + (' + local changes' if dirty else '')
    except OSError:
        return '?'


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--build', type=pathlib.Path, default=ROOT / 'build')
    parser.add_argument('--require-fresh', action='store_true')
    args = parser.parse_args()
    bad = False
    built = args.build / 'flight-store' / 'lm.reu'
    print(f'checkout {head()}; compare against the build at that checkout (rebuild with `make flight-disk` if build/ is older)'
          + (f'; build/flight-store/lm.reu written {time.strftime("%Y-%m-%d %H:%M UTC", time.gmtime(built.stat().st_mtime))}'
             if built.exists() else ''))
    pending = json.loads(PENDING.read_text()) if PENDING.exists() else {}
    for program, m in PROGRAMS.items():
        commit, moved, missing, unchecked = check(program, m, args.build)
        if missing:
            bad = True
            print(f'{program}: UNKNOWN (accepted at {commit}); missing: {", ".join(missing)}')
        elif moved and program in pending and all(
                hashlib.sha256((args.build / n).read_bytes()).hexdigest() == pending[program]['files'].get(n)
                for n in moved):
            print(f'{program}: STALE, pending re-acceptance (since {pending[program]["since"]}; accepted at {commit});'
                  f' changed since, at their pinned hashes: {", ".join(moved)}')
        elif moved:
            bad = True
            print(f'{program}: STALE (accepted at {commit}); changed since: {", ".join(moved)}'
                  + ('; the pending entry pins other bytes, so this is a new move' if program in pending else ''))
        else:
            print(f'{program}: FRESH (accepted at {commit}; every built file the doc lists matches)'
                  + (f'; not built, so not checked: {", ".join(unchecked)}' if unchecked else ''))
    return 1 if bad and args.require_fresh else 0


if __name__ == '__main__':
    sys.exit(main())
