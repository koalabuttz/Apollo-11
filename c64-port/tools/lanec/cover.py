#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Choose and check a Lane C suite's C64 rows by branch outcome.

  python3 tools/lanec/cover.py pick SUITE [--budget K]   # prints the rows
  python3 tools/lanec/cover.py check SUITE                # the frozen rows take every host arc
  python3 tools/lanec/cover.py record OUT SUITE...        # the c64_branch_outcomes record of a certificate

Runs every case of SUITE alone through build/lanec-cover/lanec-native
(the native runner built with --coverage) and reads the branch arcs it
takes in every linked source (tools/c64_row_cover.py, gcov -b). pick
prints the fewest cases whose arcs together equal the whole suite's, for
tools/lanec/freeze.py SUITE --rows; check fails unless the cases frozen in
tests/lanec_<suite>_c64.h take every arc the host cases take; record
writes, for SUITEs whose frozen rows pass check (a suite with no frozen
header runs every case on the C64), their host case and arc counts, the
frozen rows, the sources measured and the arcs the C64 rows miss (none).
Traces are kept in build/lanec-cover/<suite>-<runner digest>.json.
"""
import argparse, hashlib, json, os, pathlib, re, subprocess, sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tools'))
import c64_row_cover as rc  # noqa: E402

p = argparse.ArgumentParser()
p.add_argument('mode', choices=('pick', 'check', 'record'))
p.add_argument('args', nargs='+')
p.add_argument('--budget', type=int)
p.add_argument('--not-run', action='append', default=[], metavar='SUITE=N,N,...',
               help='record: frozen rows no C64 program runs (they do not fit one); their arcs do not count')
p.add_argument('--pool', action='store_true',
               help='record: a host arc counts as taken on the C64 when any suite\'s C64 rows take it')
a = p.parse_args()
env = dict(os.environ, SKIP_EVIDENCE_RESTORE='1')
subprocess.run(['make', '-s', 'build/lanec-cover/lanec-native'], cwd=ROOT, env=env, check=True)
objdir = ROOT / 'build/lanec-cover'
exe = objdir / 'lanec-native'
digest = hashlib.sha256(exe.read_bytes()).hexdigest()[:16]


def traced(suite):
    saved = objdir / f'{suite}-{digest}.json'
    if saved.exists():
        return rc.load(saved)
    rows = subprocess.run([str(exe), suite], cwd=ROOT, capture_output=True, check=True).stdout.count(b'\n')
    t = rc.trace(objdir, range(rows), lambda n: [str(exe), suite, str(n)])
    rc.save(saved, t)
    return t


def frozen(suite):
    path = ROOT / f'tests/lanec_{suite.replace("-", "_")}_c64.h'
    return [int(x) for x in re.findall(r'\{(\d+)u, 0x', path.read_text())] if path.exists() else None


if a.mode == 'pick':
    t = traced(a.args[0])
    host = frozenset().union(*t.values())
    chosen = rc.cover(t, budget=a.budget)
    print(','.join(map(str, chosen)))
    print(f'{a.args[0]}: {len(chosen)} of {len(t)} cases take all {len(host)} host branch arcs', file=sys.stderr)
elif a.mode == 'check':
    t = traced(a.args[0])
    host = frozenset().union(*t.values())
    rows = frozen(a.args[0])
    rc.check(t, rows, f'{a.args[0]} C64 rows')
    print(f'PASS: {a.args[0]}: the {len(rows)} C64 rows take all {len(host)} host branch arcs')
else:
    out, record = pathlib.Path(a.args[0]), {'method': (
        'tools/lanec/cover.py: every host case of each suite run alone through build/lanec-cover/lanec-native '
        '(-O0 --coverage) and the branch arcs it takes in every linked source read with gcov -b '
        '(tools/c64_row_cover.py); the C64 rows are the cases frozen in tests/lanec_<suite>_c64.h (every case '
        'when the suite has none), and their arcs are compared with the whole host suite\'s'), 'suites': {}}
    missing_all, skip, seen = [], {}, {}
    for item in a.not_run:
        suite, ns = item.split('=')
        skip[suite] = {int(n) for n in ns.split(',') if n}
    for suite in a.args[1:]:
        t = traced(suite)
        rows = frozen(suite)
        run = [n for n in (rows if rows is not None else t) if n not in skip.get(suite, ())]
        seen[suite] = (t, rows, run, frozenset().union(*t.values()), frozenset().union(*(t[n] for n in run)))
    pool = frozenset().union(*(s[4] for s in seen.values())) if a.pool else None
    for suite, (t, rows, run, host, on_c64) in seen.items():
        missing = sorted(host - (pool if a.pool else on_c64))
        missing_all += [f'{suite}: {m}' for m in missing]
        record['suites'][suite] = {'host_cases': len(t), 'c64_rows': rows if rows is not None else 'all',
                                   'host_branch_arcs': len(host), 'c64_branch_arcs': len(on_c64 & host),
                                   'sources_measured': sorted({x.split(':')[0] for x in host})}
        if suite in skip:
            record['suites'][suite]['c64_rows_not_run'] = sorted(skip[suite])
        if a.pool:
            record['suites'][suite]['c64_branch_arcs_with_other_suites'] = len(pool & host)
        print(f'{suite}: {len(run)} C64 rows take {len(on_c64 & host)} of {len(host)} host branch arcs'
              + (f', {len(pool & host)} with the other suites\' C64 rows' if a.pool else ''))
    if a.pool:
        record['method'] += ('; with --pool, a host arc of one suite counts as taken on the C64 when the C64 rows '
                             'of any suite in the record take it (the same source line and branch in the same build)')
    record['host_branches_missing_on_c64'] = missing_all
    out.write_text(json.dumps(record, indent=1) + '\n')
    if missing_all:
        sys.exit(f'FAIL: the C64 rows miss {len(missing_all)} host branch arcs')
