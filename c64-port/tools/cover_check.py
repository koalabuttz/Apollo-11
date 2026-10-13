#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Re-derive the committed C64 branch covers that certificates cite.

For every certificate in docs/port-coverage.json whose evidence names a
c64_branch_outcomes record, re-run that record's cover on the current
sources: host branch outcomes of all the frozen cases against those of
the committed C64 rows. Interpretive-routine milestones (tools/interp_specs.py)
re-run with `interp_routine_differential.py <id> --cover-check`; other
records have no re-derivation yet and are reported unverified.

Writes a vice manifest piece (tools/vice_manifest.py) carrying
`c64_cover`: {milestone: {record, verified, missing, ...}}. Exits 0: the
nightly reports, tools/cert_precheck.py judges.

  python3 tools/cover_check.py --shard 0/2 --out build/vice-manifest-cover0.json
  python3 tools/cover_check.py --only r30 cm-p11 --out /dev/stdout
"""
import argparse
import json
import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import interp_specs  # noqa: E402
from vice_manifest import CERTIFICATES  # noqa: E402


def cited():
    """{milestone: record path} for every c64_branch_outcomes a certificate cites."""
    found = {}
    for entry in json.loads((ROOT / 'docs/port-coverage.json').read_text()).get('files', []):
        for key in CERTIFICATES:
            record = ((entry.get(key) or {}).get('evidence') or {}).get('c64_branch_outcomes') or {}
            path = record.get('path') if isinstance(record, dict) else None
            if path and path.startswith('evidence/'):
                found[pathlib.PurePosixPath(path).parts[1]] = path
    return dict(sorted(found.items()))


def check(milestone, record):
    if milestone not in interp_specs.SPECS:
        return {'record': record, 'verified': False, 'reason': 'no re-derivation for this record yet'}
    fetch = subprocess.run([sys.executable, 'tools/evidence.py', 'fetch', milestone], cwd=ROOT,
                           capture_output=True, text=True)
    if fetch.returncode:
        return {'record': record, 'verified': False, 'reason': 'evidence fetch failed: ' + fetch.stdout[-300:]}
    run = subprocess.run([sys.executable, 'tools/interp_routine_differential.py', milestone, '--cover-check', record],
                         cwd=ROOT, capture_output=True, text=True)
    lines = [l for l in run.stdout.splitlines() if l.startswith('{')]
    if not lines:
        return {'record': record, 'verified': False,
                'reason': 'cover check failed: ' + (run.stderr or run.stdout)[-500:]}
    result = json.loads(lines[-1])
    result.pop('milestone', None)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--shard', metavar='K/N', default='0/1')
    parser.add_argument('--only', nargs='*')
    parser.add_argument('--name', default='cover-check')
    parser.add_argument('--out', type=pathlib.Path, required=True)
    args = parser.parse_args()
    k, n = (int(x) for x in args.shard.split('/'))
    records = cited()
    names = [m for m in records if not args.only or m in args.only][k::n]
    commit = subprocess.run(['git', 'rev-parse', 'HEAD'], cwd=ROOT, capture_output=True, text=True).stdout.strip()
    cover = {}
    for milestone in names:
        cover[milestone] = dict(check(milestone, records[milestone]), commit=commit)
        r = cover[milestone]
        print('%s %s: %s' % ('PASS' if r.get('verified') and not r.get('missing') else
                             'MISS' if r.get('verified') else 'UNVERIFIED', milestone,
                             '%d host branches missing on C64' % r.get('missing_count', 0) if r.get('verified')
                             else r.get('reason')), flush=True)
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps({'name': args.name, 'commit': commit, 'targets': {}, 'c64_cover': cover},
                                   indent=1, sort_keys=True) + '\n')
    return 0


if __name__ == '__main__':
    sys.exit(main())
