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

First lists the points the host run to the end of the C64 window bridges
to yaAGC (unregistered, or refused by their table), with file, label and
owning lane (tools/flight_holes.py): each is a stop on the C64.
--stop-on-holes fails there instead of running VICE.

Prints wall cycles per AGC second, the AGC time reached, the step status
(a refused or untranslated point stops the run: there is no yaAGC to
bridge to on the C64) and per-interrupt costs; writes
build/flight-store/flight-scenario-vice.json.

  flight_scenario_vice.py [--scenario lm-p63-descent-thrust] [--snap-cs CS] [--cs N] [--stop-on-holes] [--profile]
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
import flight_holes  # noqa: E402
import flight_host_differential as fhd  # noqa: E402


def snapshot(exe, name, cs, tag):
    scen = {s[0]: s for s in fhd.FLIGHT}[name]
    _, m, _, keys, env = scen
    header = dp.WORK / f'{tag}-snap.h'
    header.unlink(missing_ok=True)
    # The host takes the snapshot at the first point at or after cs with no
    # interrupt pending (a long job, such as PICAPAR, can hold one off for
    # over 100 cs), then stops; running to cs + 1000 leaves room for that.
    r = subprocess.run([str(exe), fhd.ROPE[m], str(cs + 1000), keys], cwd=ROOT,
                       env={**os.environ, **env, 'FLIGHT_SCALED': '1,1,2', 'FLIGHT_SNAPSHOT': f'{cs},{header}'},
                       capture_output=True, text=True)
    (dp.WORK / f'{tag}-host.log').write_text(r.stdout + r.stderr)
    if not header.exists():
        sys.exit(f'FAIL: no snapshot for {name} (see {dp.WORK}/{tag}-host.log)')
    return header


def host_holes(exe, name, cs, tag):
    """The points the host run to cs bridges, as flight_holes.describe dicts."""
    _, m, _, keys, env = {s[0]: s for s in fhd.FLIGHT}[name]
    r = subprocess.run([str(exe), fhd.ROPE[m], str(cs), keys], cwd=ROOT,
                       env={**os.environ, **env, 'FLIGHT_SCALED': '1,1,2'}, capture_output=True, text=True)
    (dp.WORK / f'{tag}-holes.log').write_text(r.stdout + r.stderr)
    words, owners = flight_holes.listing(flight_holes.MISSION[m]), flight_holes.lanes()
    return [flight_holes.describe(h, words, owners, m) for h in flight_holes.HOLE.finditer(r.stdout)]


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
    p.add_argument('--stop-on-holes', action='store_true', help='fail before VICE if the host run bridges any point')
    p.add_argument('--profile', action='store_true', help='cycles per resident step segment (RK_STEP_PROFILE)')
    a = p.parse_args()
    dp.WORK.mkdir(parents=True, exist_ok=True)
    m = {s[0]: s for s in fhd.FLIGHT}[a.scenario][1]
    # Each window's files (snapshot, PRG, VICE directory, result) carry its
    # start, so windows of one scenario can run side by side.
    tag = f'{a.scenario}-{a.snap_cs}'
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
    holes = host_holes(exe, a.scenario, a.snap_cs + a.cs, tag)
    print(f'{a.scenario}: the host run to {a.snap_cs + a.cs} cs bridges {len(holes)} points, each a stop on the C64')
    for d in holes:
        print(f"  {d['kind']:6} {d['point']} {d['label']:14} {d['file']:40} lane {d['lane']:3} {d['arrivals']:>5} arrivals"
              + (f"  refused by {d['refused_by']}" if d['refused_by'] else ''))
    if holes and a.stop_on_holes:
        (dp.OUT / 'flight-scenario-vice.json').write_text(json.dumps(dict(ok=False, host_holes=holes), indent=1) + '\n')
        return 1
    header = snapshot(exe, a.scenario, a.snap_cs, tag)
    prg = dp.link(a.cc, tag, header, a.cs, scratch, os.environ.get('FSV_STEPS') == '1', m, a.profile)
    r = dp.vice(a, tag, prg, pinned, m)
    if a.profile:
        r['profile'] = dp.profile(tag)
    r.update(scenario=a.scenario, snap_cs=a.snap_cs, requested_cs=a.cs, host_holes=holes)
    # Status 0 means no step refused; it does not mean the window ran to its
    # end (a harness exit before --cs, like the TIME1 wrap #595 fixed, also
    # reports status 0). A window that stops short of --cs without a refusal
    # fails, so a "no stop" result always covers the whole window.
    if r.get('status') == 0 and r.get('agc_cs') is not None and r['agc_cs'] < a.cs:
        r.update(ok=False, short=True)
        print(f"FAIL: {a.scenario}: status 0 but the window ended after {r['agc_cs']} of {a.cs} cs")
    (dp.OUT / 'flight-scenario-vice.json').write_text(json.dumps(r, indent=1) + '\n')
    (dp.WORK / f'{tag}.json').write_text(json.dumps(r, indent=1) + '\n')
    if os.environ.get('FSV_STEPS') == '1':
        # One record per step: the physical bank (superbank applied) and Z
        # (dap_pass_check.c DP_STEPS); fixed-fixed Z as bank 02 or 03.
        print('last steps (bank,Z/status):',
              ' '.join((f"{x['job_loads']:02o},{x['steps']:04o}" if 0o2000 <= x['steps'] < 0o4000 else
                        f"{x['steps'] >> 10:02o},{0o2000 + x['steps'] % 1024:04o}" if 0o4000 <= x['steps'] < 0o10000 else
                        f"{x['steps']:05o}")
                       + f"/{x['status']}" for x in r.get('records', [])[-12:]))
    if 'by_request' in r:
        print(f"{a.scenario} from {a.snap_cs} cs: {r['wall_cycles']} cycles for {r['agc_cs']} AGC cs "
              f"({r['wall_per_agc']} x real time), {r['dropped_ticks']} ticks dropped, {r['steps']} steps, "
              f"status {r['status']} at {r['at']} (Z {r['z']} FB {r['fb']} superbank {r['superbank']})")
        for k, b in sorted(r['by_request'].items()):
            print(f"  {k}: {b['count']} x, mean {b['cycles'] // b['count']} cycles (max {b['max_cycles']}), "
                  f"{b['steps']} steps, {b['job_loads']} job and {b['task_loads']} task loads")
        for x in r.get('profile', []):
            print(f"  {x['segment']}: {x['calls']} x, {x['cycles']} cycles"
                  f" ({x['cycles'] // x['calls'] if x['calls'] else 0} each)")
    else:
        print(f"FAIL: {a.scenario}: {r.get('error')}")
    return 0 if r.get('ok') else 1


if __name__ == '__main__':
    sys.exit(main())
