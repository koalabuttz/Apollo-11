#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""A flight scenario on the C64 (docs/FLIGHT-STORE.md, Flight scenarios):
the host flight run (tools/flight_host_run.c, scaled time x1 with the
scenario's inputs and keys) writes the native state at SNAP_CS
(FLIGHT_SNAPSHOT); platform/c64/dap_pass_check.c boots the LM flight
resident on the whole-program flight store in VICE, loads that state and
runs resident steps with CIA1 interrupts on for --cs centiseconds of AGC
time, recording cycles, steps and module loads per AGC interrupt (as
tools/dap_pass_vice.py does for the DAP). The C64 run has no keys and no
engine model: it continues from the snapshot as the AGC would with the
PIPAs still.

Prints wall cycles per AGC second, the AGC time reached, the step status
(a refused or untranslated point stops the run: there is no yaAGC to
bridge to on the C64) and per-interrupt costs; writes
build/flight-store/flight-scenario-vice.json.

  flight_scenario_vice.py [--scenario lm-p63-descent-thrust] [--snap-cs CS] [--cs N]
"""
import argparse
import json
import os
import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import dap_pass_vice as dp  # noqa: E402
import flight_host_differential as fhd  # noqa: E402


def snapshot(exe, name, cs):
    scen = {s[0]: s for s in fhd.FLIGHT}[name]
    _, m, _, keys, env = scen
    header = dp.WORK / f'{name}-snap.h'
    header.unlink(missing_ok=True)
    r = subprocess.run([str(exe), fhd.ROPE[m], str(cs + 1), keys], cwd=ROOT,
                       env={**os.environ, **env, 'FLIGHT_SCALED': '1,1,2', 'FLIGHT_SNAPSHOT': f'{cs},{header}'},
                       capture_output=True, text=True)
    (dp.WORK / f'{name}-host.log').write_text(r.stdout + r.stderr)
    if not header.exists():
        sys.exit(f'FAIL: no snapshot for {name} (see {dp.WORK}/{name}-host.log)')
    return header


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('--cc', default=os.environ.get('MOS_CC', str(ROOT / 'deps/llvm-mos/bin/mos-c64-clang')))
    p.add_argument('--host-cc', default=os.environ.get('CC', 'cc'))
    p.add_argument('--scenario', default='lm-p63-descent-thrust')
    p.add_argument('--snap-cs', type=int, default=17500, help='AGC centisecond the C64 run starts from')
    p.add_argument('--cs', type=int, default=200, help='AGC centiseconds to run on the C64')
    p.add_argument('--vice', default=os.environ.get('VICE', 'x64sc'))
    p.add_argument('--rom-dir', type=pathlib.Path,
                   default=pathlib.Path(os.environ.get('VICE_ROM_DIR', ROOT / 'deps/vice-data/C64')))
    p.add_argument('--data-dir', type=pathlib.Path,
                   default=pathlib.Path(os.environ.get('VICE_DATA_DIR', ROOT / 'deps/vice-data')))
    p.add_argument('--cycles', type=int, default=6_000_000_000)
    p.add_argument('--timeout', type=int, default=3600)
    a = p.parse_args()
    dp.WORK.mkdir(parents=True, exist_ok=True)
    m = {s[0]: s for s in fhd.FLIGHT}[a.scenario][1]
    subprocess.run([sys.executable, 'tools/flight_store_build.py', '--missions', m, '--host-cc', a.host_cc],
                   cwd=ROOT, check=True)
    subprocess.run([sys.executable, 'tools/flight_store_build.py', '--host-only', '--missions', m,
                    '--host-cc', a.host_cc], cwd=ROOT, check=True)
    exe = fhd.build(a.host_cc, m, fhd.frozen_engine(a.host_cc))
    manifest = json.loads((dp.OUT / f'{m}-manifest.json').read_text())
    pinned = any(mod['slot'] == 3 for mod in manifest['modules'])
    reu = (dp.OUT / f'{m}.reu').read_bytes()
    scratch = len(reu) - dp.SCRATCH_BYTES
    if any(reu[scratch:]):
        sys.exit(f'FAIL: no unused {dp.SCRATCH_BYTES} bytes at the end of {m}.reu')
    header = snapshot(exe, a.scenario, a.snap_cs)
    prg = dp.link(a.cc, a.scenario, header, a.cs, scratch, os.environ.get('FSV_STEPS') == '1', m)
    r = dp.vice(a, a.scenario, prg, pinned, m)
    r.update(snap_cs=a.snap_cs, requested_cs=a.cs)
    (dp.OUT / 'flight-scenario-vice.json').write_text(json.dumps(r, indent=1) + '\n')
    if os.environ.get('FSV_STEPS') == '1':
        # One record per step: the physical bank (superbank applied) and Z
        # (dap_pass_check.c DP_STEPS); the bank only where Z is switched fixed.
        print('last steps (bank,Z/status):',
              ' '.join((f"{x['job_loads']:02o},{x['steps']:04o}" if 0o2000 <= x['steps'] < 0o4000 else f"{x['steps']:05o}")
                       + f"/{x['status']}" for x in r.get('records', [])[-12:]))
    if 'by_request' in r:
        print(f"{a.scenario} from {a.snap_cs} cs: {r['wall_cycles']} cycles for {r['agc_cs']} AGC cs "
              f"({r['wall_per_agc']} x real time), {r['dropped_ticks']} ticks dropped, {r['steps']} steps, "
              f"status {r['status']} at {r['bank']},{r['z'][1:]} (Z {r['z']} FB {r['fb']} superbank {r['superbank']})")
        for k, b in sorted(r['by_request'].items()):
            print(f"  {k}: {b['count']} x, mean {b['cycles'] // b['count']} cycles (max {b['max_cycles']}), "
                  f"{b['steps']} steps, {b['job_loads']} job and {b['task_loads']} task loads")
    else:
        print(f"FAIL: {a.scenario}: {r.get('error')}")
    return 0 if r.get('ok') else 1


if __name__ == '__main__':
    sys.exit(main())
