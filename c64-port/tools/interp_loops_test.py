#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""tools/interp_routine_differential.py's step-cap loops (#664, #746).

A case the oracle runs to its cycle cap (kind 9) and the native runner ends
in failing (kind 2) is a loop only when the native run ended at its step cap
N with the same image as at 4N steps (also at the cap), and yaAGC still runs
at 4x its cycles with LOC in a routine the native loop visits. Everything
else stays a mismatch.

  tools/interp_loops_test.py          the rule, frozen_only and the proof record
                                      (mocked runners; the CI ledgers job)
  tools/interp_loops_test.py --real   also the real PERIODCH case on the built
                                      cm-conics runners and yaAGC (make
                                      interp-loops-check, host CI)
"""
import json
import os
import shutil
import sys
import tempfile
import types

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import interp_routine_differential as D  # noqa: E402

N = 2000
IMAGE = ['3103', '3107222850', '0', '1', '2']


def rec(n1=('2', N, True), n4=('2', 4 * N, True), image4=IMAGE, r4=('9', '12,2136'),
        visited=('12,2110', '12,2136')):
    return {'n1': {'kind': n1[0], 'steps': n1[1], 'capped': n1[2], 'image': IMAGE, 'visits': []},
            'n4': {'kind': n4[0], 'steps': n4[1], 'capped': n4[2], 'image': image4, 'visits': []},
            'r4': {'kind': r4[0], 'loc': r4[1]}, 'visited': set(visited), 'routine': 'cm 12,2074'}


def classify(records, max_steps=N):
    asked = []

    def probe(idx):
        asked.append(list(idx))
        return [records[i] for i in idx]
    return D.step_cap_loops(range(len(records)), max_steps, probe), asked


# The loop: both runs at the cap with one image, yaAGC still in the loop.
got, asked = classify([rec()])
assert got == {0: 'cm 12,2074'} and asked == [[0]], (got, asked)

# A kind 2 that is not the step cap stays a mismatch, even when it repeats
# exactly at 4x (a refusal: lm-p70-p71's I/O-channel stops are kind 2 too).
NEAR = (rec(n1=('2', 17, False), n4=('2', 17, False)),
        # (a refusal on the very step the cap would have come)
        rec(n1=('2', N, False), n4=('2', N, False)),
        # (the cap at N, a refusal later)
        rec(n4=('2', 3 * N, False)),
        # (the cap at N, the routine ends by itself later)
        rec(n4=('0', 3 * N, False)),
        # (both at their caps, the image moved: still making progress)
        rec(image4=IMAGE[:-1] + ['3']),
        # (yaAGC ends given 4x its cycles: a slow routine, not a loop)
        rec(r4=('0', None)),
        # (yaAGC still running, but outside the routines the native loop runs)
        rec(r4=('9', '12,3000')),
        rec(r4=('9', None)))
for bad in NEAR:
    got, _ = classify([bad])
    assert got == {}, bad

# The clamp: the runner's step cap is 16-bit, so no rule (and no probe) when
# 4N does not fit; the largest N that fits still gets one.
got, asked = classify([rec(n1=('2', 0x4000, True), n4=('2', 0x10000, True))], 0x4000)
assert got == {} and asked == [], (got, asked)
got, asked = classify([rec(n1=('2', 0x3fff, True), n4=('2', 0xfffc, True))], 0x3fff)
assert got == {0: 'cm 12,2074'} and asked == [[0]], (got, asked)

# frozen_only (cover-check, cert_arms): the same rule on the frozen cases.
# Case 0 loops in an erasable-CADR cycle (9/9), case 1 is a step-cap loop,
# case 2 a refusal paired with an oracle timeout, case 3 a kind 2 the oracle
# ends, case 4 a stop. Only 0 and 1 leave, both in its loop record; only 1
# and 2 are probed.
native = [['0', '9', '1', '0'], ['1', '2', '2', '5'], ['2', '2', '3', '6'], ['3', '2', '4', '7'], ['4', '0', '5', '8']]
oracle = {0: ['0', '9', '1', '0'], 1: ['1', '9', '2', '0'], 2: ['2', '9', '3', '0'], 3: ['3', '0', '4', '7']}
mi = types.SimpleNamespace(m='cm', cp=types.SimpleNamespace(where=lambda p: '12,%04o' % p))
FCASES = [{'cm': 1, 'point': 0o2074 + k, 'seed': k} for k in range(5)]
probed = []
saved = {k: getattr(D, k) for k in ('run', 'write_cases', 'operand_stops', 'step_cap_probe')}
tmp = None
try:
    files = {}

    def write_cases(path, missions, cases):
        files[path] = [c['seed'] for c in cases]
        open(path, 'w').close()

    def run(cmd):
        if cmd[0] == 'ref':
            return [oracle[c] for c in files[cmd[-1]]]
        if cmd[-1] == 'fb':
            return [[str(k)] for k in range(len(files[cmd[1]]))]
        return [native[c] for c in files[cmd[1]]]

    def step_cap_probe(spec_id, spec, missions, cases, path, ref, nat):
        assert spec_id == 'cm-conics' and nat == 'nat', (spec_id, nat)

        def probe(idx):
            probed.append(list(idx))
            return [rec() if cases[i]['seed'] == 1 else NEAR[0] for i in idx]
        return probe
    D.run, D.write_cases, D.step_cap_probe = run, write_cases, step_cap_probe
    D.operand_stops = lambda missions, cases, rows_n, fbrows: set()
    tmp = tempfile.mkdtemp()
    path = os.path.join(tmp, 'cases.bin')
    write_cases(path, None, FCASES)
    record = {}
    kept = D.frozen_only([None, mi], FCASES, 'ref', 'nat', path, 'cm-conics', record)
finally:
    if tmp:
        shutil.rmtree(tmp, ignore_errors=True)
    for k, v in saved.items():
        setattr(D, k, v)
assert [c['seed'] for c in kept] == [2, 3, 4] and probed == [[1, 2]], (kept, probed)
assert record == {'erasable_cadr_cycle': {'cm 12,2074': 1},
                  'step_cap': {'routines': {'cm 12,2074': 1},
                               'cases': [{'case': 1, 'seed': 1, 'start': 'cm 12,2075', 'point': 0o2075}]}}, record

# cover-check and cert_arms measure fail when the cases a rerun leaves out
# are not the ones the proof records (a proof with no 'loops' key: none).
assert D.loops_mismatch(record, {'loops': record}) == ''
assert D.loops_mismatch({}, {}) == ''
assert D.loops_mismatch(record, {}) != ''
assert D.loops_mismatch({}, {'loops': record}) != ''
assert D.loops_mismatch(record, {'loops': {'erasable_cadr_cycle': {'cm 12,2074': 1}}}) != ''

# The proof's record: every left-out case by routine, the step-cap ones also
# by case.
cases = [{'cm': 1, 'point': 0o2074, 'seed': 7}, {'cm': 1, 'point': 0o2074, 'seed': 8},
         {'cm': 1, 'point': 0o2200, 'seed': 9}]
got = D.loops_record([None, mi], cases, {2}, {0: 'cm 12,2074', 1: 'cm 12,2074'})
want = {'erasable_cadr_cycle': {'cm 12,2200': 1},
        'step_cap': {'routines': {'cm 12,2074': 2},
                     'cases': [{'case': 0, 'seed': 7, 'start': 'cm 12,2074', 'point': 0o2074},
                               {'case': 1, 'seed': 8, 'start': 'cm 12,2074', 'point': 0o2074}]}}
assert got == want, got
assert D.loops_record([None, mi], cases, set(), {}) == {}

# span_point: the oracle's LOC at the cap maps to the routine holding it,
# [point, next point) in one bank; past a bank's last point or before its
# first, no routine (#746 round 2).
PTS = [0o12131, 0o12133, 0o12136, 0o12140, 0o12141, 0o14000]
assert D.span_point(PTS, 0o12136) == 0o12136
assert D.span_point(PTS, 0o12135) == 0o12133 and D.span_point(PTS, 0o12137) == 0o12136
assert D.span_point(PTS, 0o12130) is None and D.span_point(PTS, 0o12142) is None
assert D.span_point(PTS, 0o14000) == 0o14000 and D.span_point(PTS, 0o14001) is None
assert D.span_point([], 0o12136) is None

if '--real' in sys.argv[1:]:
    # cm-conics IRC_SALT=specs1017 case 27577 on the real runners: the
    # native run at 2,000 and 8,000 steps, yaAGC at 16M cycles.
    import interp_specs
    c = json.load(open(os.path.join(D.R, 'tests', 'interp_loops_periodch.json')))
    spec = interp_specs.SPECS[c['spec']]
    os.environ.update(D.oracle_env(spec))
    missions = [D.Mission(spec, m) if m in spec['missions'] else D.Absent(m) for m in ('lm', 'cm')]
    mi = missions[c['cm']]
    point = [p for p in mi.case_points if mi.cp.where(p) == c['point']]
    assert len(point) == 1, point
    case = {'cm': c['cm'], 'point': point[0], 'seed': c['seed'], 'edits': D.Edits(c['edits']), 'ie': c['ie'],
            'stop': c['stop'], 'sb': c['sb']}
    os.makedirs(D.B, exist_ok=True)
    path = os.path.join(D.B, 'interp-loops-periodch.bin')
    D.write_cases(path, missions, [case])
    ref, nat = D.build(c['spec'], spec)
    r, n = D.run([ref, *D.ROPES, path])[0], D.run([nat, path])[0]
    assert r[1] == '9' and n[1] == '2', (r, n)
    record = D.step_cap_probe(c['spec'], spec, missions, [case], path, ref, nat)([0])[0]
    n1, n4 = record['n1'], record['n4']
    assert (n1['steps'], n1['capped'], n4['steps'], n4['capped']) == (
        spec['max_steps'], True, 4 * spec['max_steps'], True), (n1['steps'], n4['steps'])
    assert n1['image'] == n4['image'] and record['r4']['kind'] == '9', record['r4']
    assert record['r4']['loc'] in record['visited'], (record['r4'], sorted(record['visited']))
    got = D.step_cap_loops([0], spec['max_steps'], lambda idx: [record])
    assert got == {0: 'cm %s' % c['point']}, got
    os.remove(path)
    print('interp_loops_test: real PERIODCH case a step-cap loop (native %d and %d steps at the cap, one image; '
          'yaAGC still running at 4x its cycles with LOC in %s)' % (n1['steps'], n4['steps'], record['r4']['loc']))

print('interp_loops_test: step-cap loops OK (the loop; %d near-misses stay mismatches; the 16-bit clamp; '
      'frozen_only and its loop record; the proof record and its check; LOC by routine span)' % len(NEAR))
