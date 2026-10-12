#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Choose and check a Lane C suite's C64 rows by branch outcome.

  python3 tools/lanec/cover.py pick SUITE [--budget K]   # prints the rows
  python3 tools/lanec/cover.py check SUITE                # the frozen rows take every host arc

Runs every case of SUITE alone through build/lanec-cover/lanec-native
(the native runner built with --coverage) and reads the branch arcs it
takes in every linked source (tools/c64_row_cover.py, gcov -b). pick
prints the fewest cases whose arcs together equal the whole suite's, for
tools/lanec/freeze.py SUITE --rows; check fails unless the cases frozen in
tests/lanec_<suite>_c64.h take every arc the host cases take. Traces are
kept in build/lanec-cover/<suite>.json while the runner is unchanged.
"""
import argparse, hashlib, os, pathlib, re, subprocess, sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tools'))
import c64_row_cover as rc  # noqa: E402

p = argparse.ArgumentParser()
p.add_argument('mode', choices=('pick', 'check'))
p.add_argument('suite')
p.add_argument('--budget', type=int)
a = p.parse_args()
env = dict(os.environ, SKIP_EVIDENCE_RESTORE='1')
subprocess.run(['make', '-s', 'build/lanec-cover/lanec-native'], cwd=ROOT, env=env, check=True)
objdir = ROOT / 'build/lanec-cover'
exe = objdir / 'lanec-native'
rows = subprocess.run([str(exe), a.suite], cwd=ROOT, capture_output=True, check=True).stdout.count(b'\n')
digest = hashlib.sha256(exe.read_bytes()).hexdigest()[:16]
saved = objdir / f'{a.suite}-{digest}.json'
if saved.exists():
    t = rc.load(saved)
else:
    t = rc.trace(objdir, range(rows), lambda n: [str(exe), a.suite, str(n)])
    rc.save(saved, t)
host = frozenset().union(*t.values())
if a.mode == 'pick':
    chosen = rc.cover(t, budget=a.budget)
    print(','.join(map(str, chosen)))
    print(f'{a.suite}: {len(chosen)} of {len(t)} cases take all {len(host)} host branch arcs', file=sys.stderr)
else:
    name = a.suite.replace('-', '_')
    frozen = [int(x) for x in re.findall(r'\{(\d+)u, 0x', (ROOT / f'tests/lanec_{name}_c64.h').read_text())]
    rc.check(t, frozen, f'{a.suite} C64 rows')
    print(f'PASS: {a.suite}: the {len(frozen)} C64 rows take all {len(host)} host branch arcs')
