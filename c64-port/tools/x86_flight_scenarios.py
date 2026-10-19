#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""The native x86_64 flight program flying the host differential's flight
scenarios with its own vehicle models (platform/x86/vehicle_models.h),
compared with the host differential flying the same scenario
(tools/flight_host_run.c).

The two runs share the routines and the models but not the clock schedule:
the x86 program charges each point the median cost of
docs/flight-routine-mct.json, the harness's work clock (FLIGHT_SCALED=W)
each table's weight. So the reference is the harness on W and the
yardstick is the harness against itself: the same scenario on its other
clock (FLIGHT_SCALED=1,1,2, yaAGC's own cycles per routine). Near the end
of each scenario (the first sample point at or after 1,000 cs before it)
the x86 program passes when
  - its major mode equals the harness's;
  - its state vector is within SPREAD_K times the harness's own spread of
    the harness, plus RN_FLOOR / VN_FLOOR for the per-routine charges the
    spread cannot see (the largest such case measured: CM P40, VN 0.13 m/s
    against a spread of 0.004);
  - its erasable image (061-3777) differs from the harness's in at most
    WORDS_K times as many words as the harness's two clocks differ in,
    plus WORDS_PLUS. This is what sees programs that do not move the
    state vector: P52 and P57 with their marks left out differ in about
    200 words against a spread of 6 and 18.
KNOWN lists scenarios outside these bounds with the reason; they are
reported and do not fail the run. Results go to
build/x86-flight/scenarios/summary.json.

  x86_flight_scenarios.py [--host-cc CC] [--skip-build] [--drop-keys CHARS] [NAME ...]

Scenarios that need a model the x86 program does not have yet (the
rendezvous and landing radars, the NASSP P63 core, uplink, stolen time)
are left out. Needs yaAGC (deps/virtualagc) for the harness; the x86
program itself links none.
"""
import argparse, json, os, re, subprocess, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import flight_host_differential as fhd  # noqa: E402
import x86_flight_build as xb  # noqa: E402

OUT = xb.OUT / 'scenarios'
SPREAD_K, RN_FLOOR, VN_FLOOR = 4.0, 50.0, 0.15
WORDS_K, WORDS_PLUS = 1.5, 10
WORDS = range(0o61, 0o4000)
# The gyrocompass loop polls the liftoff discrete, so when the AGC sees
# liftoff depends on its clock schedule: TLIFTOFF is 10,005 cs on the
# harness's work clock, 10,022 on its other clock and 10,038 on the x86
# program's (FLIGHT_SATURN lifts off at 10,000). P11 navigates from the
# time it saw, so the offset is in the state vector from liftoff on
# (149 m at 10,051 cs against a spread of 74 m) and grows to orbit.
KNOWN = {'cm-p11': 'the x86 program sees liftoff 33 cs after the harness (TLIFTOFF 10038 against 10005; the '
                   "harness's other clock: 10022), and P11 navigates from that time"}
UNSUPPORTED = ('FLIGHT_CORE', 'FLIGHT_RR', 'FLIGHT_LR', 'FLIGHT_UPLINK', 'FLIGHT_STEAL')
# Each mission's MODREG, RN and VN (metres at 2^29, m/cs at 2^7).
STATE = {'lm': (0o1011, 0o1220, 0o1226), 'cm': (0o1011, 0o1170, 0o1176)}


def supported():
    return [row for row in fhd.FLIGHT if not any(k in row[4] for k in UNSUPPORTED)]


def ones(v):
    return -((~v) & 0o37777) if v & 0o40000 else v


def vector(w, a, scale, k):
    return [(ones(w[a + 2 * i]) * 16384 + ones(w[a + 2 * i + 1])) * 2.0 ** (scale - 28) * k for i in range(3)]


def harness_state(m, cs, keys, env, at, clock='W'):
    """The harness's native erasable image at the first sample point at or
    after centisecond at, on the given clock (FLIGHT_SCALED): (centisecond,
    2048 words), or None when it wrote no snapshot."""
    snap = OUT / f'snap-{os.getpid()}.h'
    subprocess.run([str(fhd.OUT / f'flight-host-run-{m}'), fhd.ROPE[m], str(cs)] + ([keys] if keys else []), cwd=ROOT,
                   env={**os.environ, **env, 'FLIGHT_SCALED': clock, 'FLIGHT_SNAPSHOT': f'{at},{snap}'},
                   capture_output=True, check=False)
    if not snap.exists():
        return None
    text = snap.read_text()
    snap.unlink()
    words = [int(x.rstrip('u')) for x in re.search(r'snap_words\[2048\][^{]*\{([^}]*)\}', text)[1].split(',')]
    return int(re.search(r'SNAP_CS (\d+)', text)[1]), words


def x86_state(m, keys, env, at):
    """The x86 program's erasable image at its sample for centisecond at:
    (run, centisecond, 2048 words), centisecond None when it wrote none."""
    samples = OUT / f'samples-{os.getpid()}.txt'
    r = subprocess.run([str(xb.OUT / f'x86-flight-{m}'), fhd.ROPE[m], str(at + 1)] + ([keys] if keys else []), cwd=ROOT,
                       env={**os.environ, **env, 'X86_SPEED': '0', 'X86_SAMPLES': str(samples),
                            'X86_SAMPLES_FROM': str(at)}, capture_output=True, text=True)
    line = samples.read_text().split('\n', 1)[0].split() if samples.exists() else []
    if samples.exists():
        samples.unlink()
    if len(line) != 2049:
        return r, None, None
    return r, int(line[0]), [int(v, 8) for v in line[1:]]


def distance(a, b):
    return sum((x - y) ** 2 for x, y in zip(a, b)) ** 0.5


def compare(name, m, cs, keys, env, drop=''):
    """One scenario: (summary row, passed). drop: key letters left out of
    the x86 run only (a check that the comparison sees them)."""
    modreg, rn, vn = STATE[m]
    ref = harness_state(m, cs, keys, env, cs - 1000)
    if ref is None:
        return dict(scenario=name, mission=m, line=f'{name}: the harness wrote no snapshot'), False
    at, hw = ref
    other = harness_state(m, cs, keys, env, at, '1,1,2')
    r, xcs, xw = x86_state(m, ''.join(c for c in keys if c not in drop), env, at)
    if other is None or xcs != at:
        why = 'the harness on its other clock wrote no snapshot' if other is None else \
            f'the x86 program stopped before {at} cs (exit {r.returncode}): {r.stderr.strip()[:200]}'
        return dict(scenario=name, mission=m, centisecond=at, line=f'{name}: {why}'), False
    sw = other[1]
    state = {'rn': [vector(w, rn, 29, 1.0) for w in (hw, sw, xw)], 'vn': [vector(w, vn, 7, 100.0) for w in (hw, sw, xw)]}
    d_rn, s_rn = distance(state['rn'][0], state['rn'][2]), distance(state['rn'][0], state['rn'][1])
    d_vn, s_vn = distance(state['vn'][0], state['vn'][2]), distance(state['vn'][0], state['vn'][1])
    words = sum(hw[i] != xw[i] for i in WORDS)
    spread_words = sum(hw[i] != sw[i] for i in WORDS)
    rn_tol, vn_tol = SPREAD_K * s_rn + RN_FLOOR, SPREAD_K * s_vn + VN_FLOOR
    words_tol = int(WORDS_K * spread_words) + WORDS_PLUS
    ok = r.returncode == 0 and hw[modreg] == xw[modreg] and d_rn <= rn_tol and d_vn <= vn_tol and words <= words_tol
    line = (f"{name}: at {at} cs major mode {hw[modreg]:02o}/{xw[modreg]:02o} (harness/x86), "
            f"RN {d_rn:.1f} m apart (limit {rn_tol:.0f}), VN {d_vn:.2f} m/s (limit {vn_tol:.2f}), "
            f"{words} words differ (limit {words_tol}; the harness's clocks: RN {s_rn:.1f} m, VN {s_vn:.3f} m/s, "
            f"{spread_words} words)")
    return dict(scenario=name, mission=m, centisecond=at, x86_exit=r.returncode,
                major_mode=dict(harness=f'{hw[modreg]:02o}', x86=f'{xw[modreg]:02o}'),
                rn_metres=dict(harness=[round(v, 1) for v in state['rn'][0]], x86=[round(v, 1) for v in state['rn'][2]],
                               difference=round(d_rn, 1), spread=round(s_rn, 1), limit=round(rn_tol, 1)),
                vn_mps=dict(harness=[round(v, 2) for v in state['vn'][0]], x86=[round(v, 2) for v in state['vn'][2]],
                            difference=round(d_vn, 3), spread=round(s_vn, 3), limit=round(vn_tol, 3)),
                words=dict(differ=words, spread=spread_words, limit=words_tol), inputs=env, line=line), ok


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('--host-cc', default=os.environ.get('CC', 'cc'))
    p.add_argument('--skip-build', action='store_true', help='reuse the built harness and x86 program')
    p.add_argument('--drop-keys', default='', metavar='CHARS',
                   help='leave these key letters out of the x86 run only, e.g. MJxyj (the mark buttons): should fail')
    p.add_argument('names', nargs='*', help='scenarios (default: every one the x86 program supports)')
    a = p.parse_args()
    rows = [r for r in supported() if not a.names or r[0] in a.names]
    if not a.skip_build:
        subprocess.run([sys.executable, 'tools/flight_store_build.py', '--host-only', '--host-cc', a.host_cc],
                       cwd=ROOT, check=True)
        engine = fhd.frozen_engine(a.host_cc)
        for m in ('lm', 'cm'):
            fhd.build(a.host_cc, m, engine)
            xb.build(a.host_cc, m, 'compiled')
    OUT.mkdir(parents=True, exist_ok=True)
    summary, failed = [], 0
    for name, m, cs, keys, env in rows:
        row, ok = compare(name, m, cs, keys, env, a.drop_keys)
        known = KNOWN.get(name)
        failed += not ok and not known
        row.update(ok=ok, known=known if not ok else None)
        summary.append(row)
        print(row['line'] + ('' if ok else f' KNOWN: {known}' if known else ' FAILED'), flush=True)
    (OUT / 'summary.json').write_text(json.dumps(summary, indent=1) + '\n')
    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main())
