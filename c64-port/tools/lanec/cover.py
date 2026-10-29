#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Choose and check a Lane C suite's C64 rows by branch outcome.

  python3 tools/lanec/cover.py pick SUITE [--budget K]   # prints the rows
  python3 tools/lanec/cover.py pick SUITE --keep          # the frozen rows plus the fewest that add the arcs they miss
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
p.add_argument('--keep', action='store_true', help='pick: keep the frozen rows and add rows for the arcs they miss')
p.add_argument('--not-run', action='append', default=[], metavar='SUITE=N,N,...',
               help='record: frozen rows no C64 program runs (they do not fit one); their arcs do not count')
p.add_argument('--legacy', action='append', default=[], metavar='SUITE=TARGET',
               help='record: a suite whose module does not fit a C64 program with the harness; its module runs '
                    'on the C64 in its own milestone\'s program (vice TARGET), and the host arcs only its cases take '
                    'are listed apart')
p.add_argument('--host-only', action='append', default=[], metavar='FILE:LINE=REASON',
               help='record: host arcs on this source line that no C64 program can take (REASON says why and where '
                    'the code is checked instead); they are listed apart with the reason')
p.add_argument('--pooled-only', action='append', default=[], metavar='SUITE=C64SUITE',
               help='record (with --pool): a host suite with no C64 rows of its own; C64SUITE runs its cases on the '
                    'C64 the way the C64 runs them, and its arcs count only through the pool')
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
    if a.keep:
        chosen = list(frozen(a.args[0]) or [])
        missing = host - frozenset().union(*(t[n] for n in chosen))
        while missing:
            best = max(sorted(t), key=lambda n: len(t[n] & missing))
            chosen.append(best)
            missing -= t[best]
        chosen.sort()
    else:
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
        '(-O0 --coverage, with LANEC_COVER_SITES so each call of a shared helper (lanec_ccs, lanec_ccs_a, '
        'lanec_ts, lanec_ev_zero, lanec_ev_nonpositive, lanec_ev_mp) branches on its outcome at the call line) '
        'and the branch arcs it takes in every linked source read with gcov -b '
        '(tools/c64_row_cover.py); the C64 rows are the cases frozen in tests/lanec_<suite>_c64.h (every case '
        'when the suite has none), and their arcs are compared with the whole host suite\'s'), 'suites': {}}
    missing_all, skip, seen = [], {}, {}
    for item in a.not_run:
        suite, ns = item.split('=')
        skip[suite] = {int(n) for n in ns.split(',') if n}
    pooled_only = dict(item.split('=') for item in a.pooled_only)
    for suite in a.args[1:]:
        t = traced(suite)
        rows = [] if suite in pooled_only else frozen(suite)
        run = [n for n in (rows if rows is not None else t) if n not in skip.get(suite, ())]
        seen[suite] = (t, rows, run, frozenset().union(*t.values()), frozenset().union(*(t[n] for n in run)))
    legacy = dict(item.split('=') for item in a.legacy)
    for suite in legacy:
        t, rows, _, host, _ = seen[suite]
        seen[suite] = (t, rows, [], host, frozenset())
    pool = frozenset().union(*(s[4] for s in seen.values())) if a.pool else None
    legacy_arcs = frozenset().union(*(seen[s][3] for s in legacy)) - (pool or frozenset())
    host_only = dict(item.split('=', 1) for item in a.host_only)
    elsewhere, apart = [], []
    for suite, (t, rows, run, host, on_c64) in seen.items():
        missing = sorted(host - (pool if a.pool else on_c64))
        elsewhere += [f'{suite}: {m}' for m in missing if m in legacy_arcs]
        apart += [f'{suite}: {m}' for m in missing if m not in legacy_arcs and m.split(' branch ')[0] in host_only]
        missing_all += [f'{suite}: {m}' for m in missing
                        if m not in legacy_arcs and m.split(' branch ')[0] not in host_only]
        record['suites'][suite] = {'host_cases': len(t), 'c64_rows': rows if rows is not None else 'all',
                                   'host_branch_arcs': len(host), 'c64_branch_arcs': len(on_c64 & host),
                                   'sources_measured': sorted({x.split(':')[0] for x in host})}
        if suite in skip:
            record['suites'][suite]['c64_rows_not_run'] = sorted(skip[suite])
        if suite in pooled_only:
            record['suites'][suite]['c64_rows_in'] = pooled_only[suite]
        if suite in legacy:
            record['suites'][suite]['c64_rows'] = []
            record['suites'][suite]['c64_program'] = legacy[suite]
        if a.pool:
            record['suites'][suite]['c64_branch_arcs_with_other_suites'] = len(pool & host)
        print(f'{suite}: {len(run)} C64 rows take {len(on_c64 & host)} of {len(host)} host branch arcs'
              + (f', {len(pool & host)} with the other suites\' C64 rows' if a.pool else ''))
    if a.pool:
        record['method'] += ('; with --pool, a host arc of one suite counts as taken on the C64 when the C64 rows '
                             'of any suite in the record take it (the same source line and branch in the same build)')
    if pooled_only:
        record['pooled_only'] = (
            'these host suites have no C64 rows of their own: the suite named in c64_rows_in runs the same cases '
            'one routine at a time through the flight modules as the C64 module test does, and its frozen rows '
            'run on the C64 (make vice-lanec-pin-chains); their arcs count as taken on the C64 only through the pool')
    if legacy:
        record['legacy_c64_programs'] = (
            'the modules of these suites do not fit one C64 program with the Lane C harness; each runs on the C64 '
            'in its own milestone\'s program (the vice target named), checked there against its own Virtual AGC '
            'vectors, and the host arcs below are taken on the C64 only there')
        record['host_branches_on_c64_only_in_legacy_programs'] = elsewhere
    if host_only:
        record['host_only_lines'] = host_only
        record['host_branches_not_on_c64'] = apart
    record['host_branches_missing_on_c64'] = missing_all
    out.write_text(json.dumps(record, indent=1) + '\n')
    if missing_all:
        sys.exit(f'FAIL: the C64 rows miss {len(missing_all)} host branch arcs')
