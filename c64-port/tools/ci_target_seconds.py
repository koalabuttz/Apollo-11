#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Rebuild tools/ci_target_seconds.json, the per-target seconds ci_run.py
--shard balances CI shards by.

  python3 tools/ci_target_seconds.py c64 <ci-summary.json|job log> [...]
  python3 tools/ci_target_seconds.py host <ci-summary.json|job log> [...]

Inputs are the build/ci-summary.json files a full port-ci run uploads (the
c64N-logs / hostN-logs artifacts) or the jobs' logs ("PASS <target> (Ns)"
lines). Each target keeps the largest time seen; targets of the other job
are left as they are.
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
    if len(sys.argv) < 3 or sys.argv[1] not in ('c64', 'host'):
        print(__doc__, file=sys.stderr)
        return 2
    job = sys.argv[1]
    seen = {}
    for path in sys.argv[2:]:
        for target, seconds in read(path).items():
            seen[target] = max(seen.get(target, 0.0), seconds)
    data = json.loads(OUT.read_text()) if OUT.exists() else {}
    data[job] = {t: round(seen[t]) for t in sorted(seen)}
    OUT.write_text(json.dumps(data, indent=1, sort_keys=True) + '\n')
    print(f'{OUT.relative_to(ROOT)}: {len(seen)} {job} targets, {sum(seen.values()) / 60:.0f} min in all')
    return 0


if __name__ == '__main__':
    sys.exit(main())
