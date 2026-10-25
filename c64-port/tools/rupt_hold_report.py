#!/usr/bin/env python3
"""The longest an interrupt request waits for a native boundary (#838), per
replay window, on the host.

The real AGC takes an interrupt request between any two instructions. The
port takes it only at a routine boundary, and since #825 only once a job's
part chain has run, so a long routine holds it. tools/flight_host_run.c
meters that wait on the replay window's work clock (FLIGHT_HOLD,
include/rupt_hold.h). The C64 runs the same meter with
`tools/flight_scenario_vice.py --replay --hold`; where a window matches
the host, the two report the same hold.

  rupt_hold_report.py [--programs Luminary099,Comanche055] [--jobs N]
                      [--out docs/acceptance] [--bound MCT]

For each program's replay windows (docs/acceptance/<program>-program-map.json
replay_windows) it runs the host to the window's end and writes
<out>/<program>-interrupt-hold.json: each window's longest hold (MCT, and
TIME1 there) and the program's largest. --bound fails the run when a window
holds a request longer than MCT (default the acceptance docs' bound,
flight_scenario_vice.HOLD_BOUND_MCT).

Host only: no VICE.
"""
import argparse
import concurrent.futures
import json
import os
import pathlib
import re
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import dap_pass_vice as dp  # noqa: E402
import flight_host_differential as fhd  # noqa: E402
import flight_scenario_vice as fsv  # noqa: E402

MISSION = {'Luminary099': 'lm', 'Comanche055': 'cm'}
HOLD = re.compile(r'^interrupt hold: max (\d+) MCT at TIME1 ([0-7]+)', re.M)
# One MCT is 11.72 microseconds.
MCT_MS = 0.01172


def window(exe, name, snap, cs):
    tag = f'hold-{name}-{snap}'
    downrupt = fhd.scenario(name)[4].get('FLIGHT_DOWNRUPT', '0') not in ('', '0')
    try:
        fsv.snapshot(exe, name, snap, tag, replay=cs, downrupt=downrupt)
    except SystemExit as e:
        return dict(scenario=name, snap_cs=snap, cs=cs, error=str(e))
    m = HOLD.search((dp.WORK / f'{tag}-host.log').read_text())
    if not m:
        return dict(scenario=name, snap_cs=snap, cs=cs, error='no interrupt hold line in the host log')
    return dict(scenario=name, snap_cs=snap, cs=cs, hold_mct=int(m[1]), time1=m[2])


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument('--programs', default='Luminary099,Comanche055')
    p.add_argument('--jobs', type=int, default=os.cpu_count() or 2)
    p.add_argument('--host-cc', default='cc')
    p.add_argument('--out', type=pathlib.Path, default=ROOT / 'docs/acceptance')
    p.add_argument('--bound', type=int, default=fsv.HOLD_BOUND_MCT,
                   help='fail on a window that holds a request longer (MCT; default %(default)s, 0: no bound)')
    a = p.parse_args()
    dp.WORK.mkdir(parents=True, exist_ok=True)
    bad = 0
    for program in a.programs.split(','):
        m = MISSION[program]
        windows = json.loads((ROOT / f'docs/acceptance/{program}-program-map.json').read_text())['replay_windows']
        # As flight_scenario_vice.py builds them: the C64 store first (the
        # host charges its weights), then the host tables.
        subprocess.run([sys.executable, 'tools/flight_store_build.py', '--missions', m, '--host-cc', a.host_cc],
                       cwd=ROOT, check=True)
        subprocess.run([sys.executable, 'tools/flight_store_build.py', '--host-only', '--missions', m,
                        '--host-cc', a.host_cc], cwd=ROOT, check=True)
        exe = fhd.build(a.host_cc, m, fhd.frozen_engine(a.host_cc))
        seen = sorted({(w[0], w[1], w[2]) for w in windows})
        with concurrent.futures.ThreadPoolExecutor(a.jobs) as pool:
            rows = list(pool.map(lambda w: window(exe, *w), seen))
        held = [r for r in rows if 'hold_mct' in r]
        worst = max(held, key=lambda r: r['hold_mct']) if held else None
        for r in rows:
            if 'error' in r:
                print(f"{program}: {r['scenario']} {r['snap_cs']}+{r['cs']}: {r['error']}")
            else:
                print(f"{program}: {r['scenario']} {r['snap_cs']}+{r['cs']}: hold {r['hold_mct']} MCT "
                      f"({r['hold_mct'] * MCT_MS:.2f} ms) at TIME1 {r['time1']}")
        if worst:
            print(f"{program}: longest hold {worst['hold_mct']} MCT ({worst['hold_mct'] * MCT_MS:.2f} ms), "
                  f"{worst['scenario']} {worst['snap_cs']}+{worst['cs']}; {len(held)} of {len(rows)} windows measured")
        over = [r for r in held if a.bound and r['hold_mct'] > a.bound]
        for r in over:
            print(f"FAIL: {program}: {r['scenario']} {r['snap_cs']}+{r['cs']} holds a request "
                  f"{r['hold_mct']} MCT, over the {a.bound} MCT bound")
        bad += len(over)
        out = dict(program=program, about=__doc__.split('\n\n')[0], mct_ms=MCT_MS,
                   longest=worst, windows=rows)
        a.out.mkdir(parents=True, exist_ok=True)
        (a.out / f'{program}-interrupt-hold.json').write_text(json.dumps(out, indent=1) + '\n')
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())
