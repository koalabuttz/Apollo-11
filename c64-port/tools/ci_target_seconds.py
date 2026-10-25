#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Rebuild tools/ci_target_seconds.json, the per-target seconds ci_run.py
--shard balances CI shards by.

  python3 tools/ci_target_seconds.py c64 <ci-summary.json|job log> [...]
  python3 tools/ci_target_seconds.py host <ci-summary.json|job log> [...]

  python3 tools/ci_target_seconds.py --replace c64 <...>

Inputs are the build/ci-summary.json files a full port-ci run uploads (the
c64N-logs / hostN-logs artifacts) or the jobs' logs ("PASS <target> (Ns)"
lines). Each target keeps the largest time seen. The measured targets are
merged into the job's table by key: targets not in the inputs keep their
seconds. --replace sets the job's table to just the measured targets (a full
run's refresh, which also drops targets that no longer exist).

The per-PR files in tools/ci_target_seconds.d/ are folded in first and then
removed (git rm them in the refresh commit). Name the CI runs in the commit
message; the file holds only seconds, one sorted key per line, so refreshes
and PRs touch only the lines of the targets they change.
"""
import json
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
OUT = ROOT / 'tools/ci_target_seconds.json'
LINE = re.compile(r'\b(?:PASS|FAIL) (\S+) \((\d+)s\)')


def read(path):
    text = pathlib.Path(path).read_text(errors='replace')
    try:
        return {r['target']: float(r['seconds']) for r in json.loads(text)['results']}
    except (ValueError, KeyError, TypeError):
        return {m[1]: float(m[2]) for m in LINE.finditer(text)}


def main():
    args = sys.argv[1:]
    replace = args[:1] == ['--replace']
    args = args[replace:]
    if len(args) < 2 or args[0] not in ('c64', 'host'):
        print(__doc__, file=sys.stderr)
        return 2
    job = args[0]
    seen = {}
    for path in args[1:]:
        for target, seconds in read(path).items():
            seen[target] = max(seen.get(target, 0.0), seconds)
    data = json.loads(OUT.read_text()) if OUT.exists() else {}
    data.pop('source_runs', None)  # run ids go in the commit message
    overlays = sorted(OUT.with_suffix('.d').glob('*.json'))
    for path in overlays:
        for j, table in json.loads(path.read_text()).items():
            data.setdefault(j, {}).update(table)
    table = {} if replace else data.get(job, {})
    table.update({t: round(v) for t, v in seen.items()})
    data[job] = table
    OUT.write_text(json.dumps(data, indent=1, sort_keys=True) + '\n')
    for path in overlays:
        path.unlink()
        print(f'folded in and removed {path.relative_to(ROOT)}')
    print(f'{OUT.relative_to(ROOT)}: {len(seen)} {job} targets measured, {len(table)} in the table, '
          f'{sum(table.values()) / 60:.0f} min in all')
    return 0


if __name__ == '__main__':
    sys.exit(main())
