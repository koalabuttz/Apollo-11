#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""The native x86_64 flight program flying the host differential's flight
scenarios with its own vehicle models (platform/x86/vehicle_models.h),
compared with the host differential flying the same scenario on the same
work clock (tools/flight_host_run.c, FLIGHT_SCALED=W).

The two runs share the routines and the models but not the exact clock
schedule (the x86 program charges each point the median cost of
docs/flight-routine-mct.json; the harness's work clock charges each table's
weight), so their states are not equal word for word. For each scenario
this checks, at a centisecond near its end (the harness's FLIGHT_SNAPSHOT
point and the x86 program's X86_SAMPLES line for the same centisecond):
the same major mode, the state vector RN within RN_TOL metres and VN within
VN_TOL m/s. Results go to build/x86-flight/scenarios/summary.json.

  x86_flight_scenarios.py [--host-cc CC] [--skip-build] [NAME ...]

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
RN_TOL, VN_TOL = 1000.0, 2.0
UNSUPPORTED = ('FLIGHT_CORE', 'FLIGHT_RR', 'FLIGHT_LR', 'FLIGHT_UPLINK', 'FLIGHT_STEAL')
# Each mission's MODREG, RN and VN (metres at 2^29, m/cs at 2^7).
STATE = {'lm': (0o1011, 0o1220, 0o1226), 'cm': (0o1011, 0o1170, 0o1176)}


def supported():
    return [row for row in fhd.FLIGHT if not any(k in row[4] for k in UNSUPPORTED)]


def ones(v):
    return -((~v) & 0o37777) if v & 0o40000 else v


def vector(w, a, scale, k):
    return [(ones(w[a + 2 * i]) * 16384 + ones(w[a + 2 * i + 1])) * 2.0 ** (scale - 28) * k for i in range(3)]


def harness_state(m, cs, keys, env, at):
    """The harness's native erasable image at the first sample point at or
    after centisecond at, on the work clock: (centisecond, 2048 words)."""
    snap = OUT / f'snap-{os.getpid()}.h'
    subprocess.run([str(fhd.OUT / f'flight-host-run-{m}'), fhd.ROPE[m], str(cs)] + ([keys] if keys else []), cwd=ROOT,
                   env={**os.environ, **env, 'FLIGHT_SCALED': 'W', 'FLIGHT_SNAPSHOT': f'{at},{snap}'},
                   capture_output=True, check=False)
    text = snap.read_text()
    snap.unlink()
    words = [int(x.rstrip('u')) for x in re.search(r'snap_words\[2048\][^{]*\{([^}]*)\}', text)[1].split(',')]
    return int(re.search(r'SNAP_CS (\d+)', text)[1]), words


def x86_state(m, keys, env, at):
    """The x86 program's erasable image at its sample for centisecond at."""
    samples = OUT / f'samples-{os.getpid()}.txt'
    r = subprocess.run([str(xb.OUT / f'x86-flight-{m}'), fhd.ROPE[m], str(at + 1)] + ([keys] if keys else []), cwd=ROOT,
                       env={**os.environ, **env, 'X86_SPEED': '0', 'X86_SAMPLES': str(samples),
                            'X86_SAMPLES_FROM': str(at)}, capture_output=True, text=True)
    line = samples.read_text().split('\n', 1)[0].split()
    samples.unlink()
    return r, int(line[0]), [int(v, 8) for v in line[1:]]


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('--host-cc', default=os.environ.get('CC', 'cc'))
    p.add_argument('--skip-build', action='store_true', help='reuse the built harness and x86 program')
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
        modreg, rn, vn = STATE[m]
        at, hw = harness_state(m, cs, keys, env, cs - 1000)
        r, xcs, xw = x86_state(m, keys, env, at)
        h_rn, x_rn = vector(hw, rn, 29, 1.0), vector(xw, rn, 29, 1.0)
        h_vn, x_vn = vector(hw, vn, 7, 100.0), vector(xw, vn, 7, 100.0)
        d_rn = sum((h - x) ** 2 for h, x in zip(h_rn, x_rn)) ** 0.5
        d_vn = sum((h - x) ** 2 for h, x in zip(h_vn, x_vn)) ** 0.5
        ok = r.returncode == 0 and xcs == at and hw[modreg] == xw[modreg] and d_rn <= RN_TOL and d_vn <= VN_TOL
        failed += not ok
        row = dict(scenario=name, mission=m, centisecond=at, x86_exit=r.returncode,
                   major_mode=dict(harness=f'{hw[modreg]:02o}', x86=f'{xw[modreg]:02o}'),
                   rn_metres=dict(harness=[round(v, 1) for v in h_rn], x86=[round(v, 1) for v in x_rn], difference=round(d_rn, 1)),
                   vn_mps=dict(harness=[round(v, 2) for v in h_vn], x86=[round(v, 2) for v in x_vn], difference=round(d_vn, 3)),
                   inputs=env, ok=ok)
        summary.append(row)
        print(f"{name}: at {at} cs major mode {row['major_mode']['harness']}/{row['major_mode']['x86']} (harness/x86), "
              f"RN {d_rn:.1f} m apart, VN {d_vn:.2f} m/s apart" + ('' if ok else ' FAILED'), flush=True)
    (OUT / 'summary.json').write_text(json.dumps(summary, indent=1) + '\n')
    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main())
