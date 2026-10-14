#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Whole-program host differential (docs/FLIGHT-STORE.md): build
tools/flight_host_run.c for both missions over every lane's routine tables
(tools/flight_store_build.py --host-only), run each scenario against yaAGC
and write the logs, and the unregistered or refused points by file and lane
(tools/flight_holes.py), to build/flight-store/differential/ (not
milestone evidence: phase 1 adds no certificate).

Scaled time (docs/RESIDENT-FLIGHT.md): each SCALED scenario runs with
FLIGHT_SCALED, yaAGC's clock frozen and the AGC clock driven by a modeled
C64 tick schedule (x1, x0.5, forced overrun); every routine must match
yaAGC under that schedule.

  flight_host_differential.py [--host-cc CC] [--skip-index]
"""
import argparse, json, os, re, subprocess, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import flight_holes  # noqa: E402

YAAGC = ROOT / 'deps/virtualagc/yaAGC'
OUT = ROOT / 'build/flight-store'
EVIDENCE = OUT / 'differential'
ROPE = {'lm': 'evidence/fixed-data-lm-original.rope.bin', 'cm': 'evidence/fixed-data-cm-original.rope.bin'}
# (mission, centiseconds, DSKY keys)
SCENARIOS = [('lm', 4000, ''), ('cm', 4000, ''),
             ('lm', 3000, 'V37E00E'), ('cm', 3000, 'V37E00E'),
             ('lm', 3000, 'V37E12E'), ('cm', 3000, 'V37E06E'), ('lm', 3000, 'V37E63E')]
# Flight scenarios (tools/flight_scenarios/): (name, mission, centiseconds,
# keys, inputs), each run under scaled time x1 with no swap cost
# (FLIGHT_SWAP_MCT=0, the DAP pinned): unscaled, with the IMU operating,
# yaAGC's TIME3 ticks between WAITLIST's two reads of it, which no native
# routine can match; with 20 ms swaps the DAP starves the CPU and no key is
# taken. lm-p63-descent: the IMU operating, PGNCS, AUTO and auto throttle,
# the LR antenna in position 1, the P63 load at 20 s; V37E63E, proceed on
# the TIG display (V06N61), ENTER on fine align (R51), proceed and then
# ENTER on the attitude maneuver (R60), proceed on V99N62; ignition at
# 146 s, the LLGE every 2 s from 172 s. No engine model: the PIPAs stay
# still, so DVMON flashes V97 (thrust fail) while guidance runs.
# lm-p63-descent-thrust: the same with FLIGHT_ENGINE (a 15,100 kg LM, the
# descent engine thrusting along the DAP's desired attitude), so DVMON
# passes and the LLGE throttles and steers the braking phase (on yaAGC it
# reaches P64 at 787 s and 4,000 ft at 900 s).
P63 = {'FLIGHT_CH30': '036357', 'FLIGHT_CH31': '057777', 'FLIGHT_CH33': '077737',
       'FLIGHT_LOAD': '2000:tools/flight_scenarios/lm-p63.load', 'FLIGHT_SWAP_MCT': '0'}
P63_KEYS = '@2200V37E63E@3400V33E@3800E@4400V33E@9600E@14300V33E'
# cm-entry: the IMU operating, the entry load at 20 s (EI at 170 s);
# V37E61E, proceed on P61's N61, N60 and N63, on P62's separation request
# (V50N25 00041) and its N61; FLIGHT_AERO (a 355 kg/m^2 capsule) gives the
# drag, so P63 senses 0.05 g and P64 and P67 guide the roll to the splash
# point (on yaAGC: P64 at 204 s, P67 at 306 s, peak 6.7 g, drogue
# conditions at 470 s about 1 nm from the target).
ENTRY = {'FLIGHT_CH30': '037377', 'FLIGHT_LOAD': '2000:tools/flight_scenarios/cm-entry.load',
         'FLIGHT_SWAP_MCT': '0', 'FLIGHT_AERO': '355'}
ENTRY_KEYS = '@2200V37E61E@4800V33E@5400V33E@6000V33E@6800V33E@7400V33E'
FLIGHT = [('lm-p63-descent', 'lm', 25000, P63_KEYS, P63),
          ('lm-p63-descent-thrust', 'lm', 30000, P63_KEYS, {**P63, 'FLIGHT_ENGINE': '15100'}),
          ('cm-entry', 'cm', 32000, ENTRY_KEYS, ENTRY)]
# Scaled time: (mission, centiseconds, keys, FLIGHT_SCALED=K,D,LAG[,SEED],
# name). x1: the C64 as fast as the AGC; x0.5: the half-speed build
# (RK_TIME_DIVISOR=2) on a C64 1.7 times slower than the AGC (the measured
# LM DAP load); overrun: four times slower with each span's cost jittered
# from 1/4 to 4 times, so most boundaries drop ticks; work: the work clock
# (FLIGHT_SCALED=W), each routine's calibrated weight, no tick dropped.
# lm-dap: the DAP running (IMU operate, PGNCS control, AUTO; CDUY moved at
# 15 s to drive its jets), T5RUPT and T6RUPT every period; no swap cost
# (FLIGHT_SWAP_MCT=0: with 20 ms swaps the DAP starves the keys and V37E12E
# never reaches P12).
DAP = {'FLIGHT_CH30': '036377', 'FLIGHT_CH31': '057777', 'FLIGHT_POKE': '1500:035=02000', 'FLIGHT_SWAP_MCT': '0'}
SCALED = [(m, cs, keys, env, sched, tag)
          for m, cs, keys, env in (('lm', 3000, 'V37E12E', {}), ('lm', 3000, 'V37E12E', DAP), ('cm', 3000, 'V37E06E', {}))
          for sched, tag in (('1,1,2', 'x1'), ('1.7,2,2', 'x0.5'), ('4,1,2,1', 'overrun'), ('W', 'work'))]


def frozen_engine(cc):
    """build/agc_engine_hardware_dv.c with AgcFrozenClock: when set, the
    scaler (and with it every counter timer and the cycles they take) and
    the IMU CDU drive stop. Unset, the engine is unchanged."""
    src = (ROOT / 'build/agc_engine_hardware_dv.c').read_text()
    loop = '  while (State->ScalerCounter >= SCALER_OVERFLOW)\n'
    assert src.count(loop) == 1
    src = src.replace(loop, '  if (AgcFrozenClock)\n    State->ScalerCounter = 0;\n'
                      '  while (!AgcFrozenClock && State->ScalerCounter >= SCALER_OVERFLOW)\n')
    # The IMU CDU drive bursts on yaAGC's cycle counter, a hardware clock
    # too: under the frozen clock it stops with the scaler.
    burst = '  if (i != 0 && (State->CycleCounter - ImuCduCount) >= IMUCDU_BURST_CYCLES)'
    assert src.count(burst) == 1
    src = src.replace(burst, burst.replace('if (', 'if (!AgcFrozenClock && ', 1))
    head = '#include "agc_engine.h"\n'
    assert src.count(head) == 1
    src = src.replace(head, head + 'int AgcFrozenClock;\n')
    OUT.mkdir(parents=True, exist_ok=True)
    c, o = OUT / 'agc_engine_scaled.c', OUT / 'agc_engine_scaled.o'
    c.write_text(src)
    subprocess.run([cc, '-O2', f'-I{YAAGC}', '-c', str(c), '-o', str(o)], cwd=ROOT, check=True)
    return o


def build(cc, m, engine):
    exe = OUT / f'flight-host-run-{m}'
    obj = OUT / f'flight_host_run_{m}.o'
    subprocess.run([cc, '-std=c99', '-O1', '-g', '-Wall', '-Wextra', '-Werror', '-Iinclude', '-Itests', f'-I{YAAGC}',
                    f'-DRK_{m.upper()}=1', f'-DFLIGHT_HOST_TABLES="../build/flight-store/{m}-host.inc"',
                    f'-DFLIGHT_HOST_SITES="../build/flight-store/{m}-sites.inc"',
                    '-c', 'tools/flight_host_run.c', '-o', str(obj)], cwd=ROOT, check=True)
    objs = sorted(str(p) for p in (OUT / 'host').glob('*.o'))
    subprocess.run([cc, str(obj), *objs, str(engine), str(YAAGC / 'agc_engine_init.o'),
                    str(YAAGC / 'rfopen.o'), '-Wl,--gc-sections', '-lm', '-o', str(exe)], cwd=ROOT, check=True)
    return exe


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('--host-cc', default=os.environ.get('CC', 'cc'))
    p.add_argument('--skip-index', action='store_true', help='reuse build/flight-store/<m>-host.inc and host objects')
    a = p.parse_args()
    if not a.skip_index:
        subprocess.run([sys.executable, 'tools/flight_store_build.py', '--host-only', '--host-cc', a.host_cc],
                       cwd=ROOT, check=True)
    engine = frozen_engine(a.host_cc)
    exe = {m: build(a.host_cc, m, engine) for m in ('lm', 'cm')}
    EVIDENCE.mkdir(parents=True, exist_ok=True)
    words = {m: flight_holes.listing(flight_holes.MISSION[m]) for m in exe}
    lanes = flight_holes.lanes()
    summary, failed = [], 0
    for m, cs, keys in SCENARIOS:
        name, env = f'{m}-{keys or "p00-idle"}', {}
        r = subprocess.run([str(exe[m]), ROPE[m], str(cs)] + ([keys] if keys else []), cwd=ROOT,
                           capture_output=True, text=True)
        (EVIDENCE / f'{name}.log').write_text(r.stdout)
        if r.returncode:
            failed += 1
        text = r.stdout
        s = re.search(r'(\d+) samples: (\d+) equal, (\d+) differ', text)
        mode = re.search(r'native (\d+), yaAGC (\d+)', text)
        misses = re.findall(r'^  (?:basic |interp) (\d\d,\d{4}) (\S+): (\d+) times; (.*)$', text, re.M)
        holes = [flight_holes.describe(h, words[m], lanes, m) for h in flight_holes.HOLE.finditer(text)]
        swaps = re.search(r'^module loads \((\d+) in (\d+) cs\)', text, re.M)
        loads = sorted((dict(module=x[0], slot=int(x[1]), kind=x[2].strip(), point=x[3],
                             label=flight_holes.label(words[m], x[3]), count=int(x[4]))
                        for x in re.findall(r'^  load (\S+) slot (\d) (basic |interp) (\d\d,\d{4}): (\d+)$', text, re.M)),
                       key=lambda d: -d['count'])
        row = dict(scenario=name, mission=m, centiseconds=cs, keys=keys, inputs=env, exit=r.returncode,
                   samples=int(s[1]) if s else 0, equal=int(s[2]) if s else 0, differ=int(s[3]) if s else None,
                   major_mode=dict(native=mode[1], yaagc=mode[2]) if mode else None,
                   routine_mismatches=[dict(point=x[0], table=x[1], times=int(x[2]), detail=x[3]) for x in misses],
                   unregistered=holes,
                   module_loads=int(swaps[1]) if swaps else None,
                   loads_per_second=round(int(swaps[1]) * 100 / max(int(swaps[2]), 1), 1) if swaps else None,
                   loads_by_point=loads)
        summary.append(row)
        print(f"{name}: {row['equal']}/{row['samples']} samples equal, {len(row['routine_mismatches'])} routines "
              f"yaAGC did not match, {len(holes)} unregistered or refused points, "
              f"{row['loads_per_second']} module loads per second"
              + (f", major mode native {mode[1]} yaAGC {mode[2]}" if mode else '') + ('' if not r.returncode else ' FAILED'))
    for name, m, cs, keys, env, sched in ([(f'{m}-{keys or "p00-idle"}{"-dap" if env else ""}-scaled-{tag}', m, cs, keys, env, sched)
                                           for m, cs, keys, env, sched, tag in SCALED]
                                          + [(f'{name}-scaled-x1', m, cs, keys, env, '1,1,2') for name, m, cs, keys, env in FLIGHT]):
        r = subprocess.run([str(exe[m]), ROPE[m], str(cs)] + ([keys] if keys else []), cwd=ROOT,
                           capture_output=True, text=True, env={**os.environ, **env, 'FLIGHT_SCALED': sched})
        (EVIDENCE / f'{name}.log').write_text(r.stdout)
        failed += bool(r.returncode)
        text = r.stdout
        t = re.search(r'(\d+) CIA interrupts, (\d+) clock periods taken, (\d+) dropped \(rk_dilation\); AGC time (\d+) cs', text)
        n = re.search(r'(\d+) native routines \((\d+) yaAGC instructions of time\), (\d+) bridged', text)
        misses = re.findall(r'^  (?:basic |interp) (\d\d,\d{4}) (\S+): (\d+) times; (.*)$', text, re.M)
        row = dict(scenario=name, mission=m, centiseconds=cs, keys=keys, inputs=env, schedule=sched, exit=r.returncode,
                   cia_interrupts=int(t[1]) if t else None, periods_taken=int(t[2]) if t else None,
                   periods_dropped=int(t[3]) if t else None, agc_centiseconds=int(t[4]) if t else None,
                   native_routines=int(n[1]) if n else None, bridged_instructions=int(n[3]) if n else None,
                   routine_mismatches=[dict(point=x[0], table=x[1], times=int(x[2]), detail=x[3]) for x in misses])
        summary.append(row)
        print(f"{name}: {row['native_routines']} routines, {len(misses)} yaAGC did not match under the schedule; "
              f"{row['periods_taken']} periods taken, {row['periods_dropped']} dropped of {row['cia_interrupts']} "
              f"interrupts" + ('' if not r.returncode else ' FAILED'))
    (EVIDENCE / 'summary.json').write_text(json.dumps(summary, indent=1) + '\n')
    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main())
