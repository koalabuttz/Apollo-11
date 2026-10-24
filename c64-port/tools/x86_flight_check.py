#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""The native x86_64 flight program's acceptance (docs/X86-FLIGHT.md):
build it (tools/x86_flight_build.py) and its lockstep check
(tools/x86_flight_lockstep.c, which links yaAGC with its clock frozen), run
the flight scenarios in both interpretive modes, and fail on any routine
yaAGC does not match, any basic point no routine holds, or a run that does
not reach its time. Each run's log, the program's own report (the points the
interpreter core ran) and summary.json go to build/x86-flight/check/.

  x86_flight_check.py [--host-cc CC] [--skip-index]
"""
import argparse, json, os, re, subprocess, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import flight_host_differential as fhd  # noqa: E402
import x86_flight_build as xb  # noqa: E402

CHECK = xb.OUT / 'check'
# (name, mission, centiseconds, keys, inputs): P00 idle on both missions;
# the LM DAP holding attitude (IMU operate, PGNCS control, AUTO; CDUY moved
# at 15 s, tools/flight_host_differential.py's DAP inputs); and the CM's
# V37E00E and V37E06E, whose interpretive code (STATEUP, INTEGRATION
# INITIALIZATION) runs compiled or through the interpreter core; P00 with a
# DOWNRUPT every 20 ms (the downlink words checked against yaAGC); and
# hardware restarts with DOWNRUPT: LM P00 at 20 s, and CM V37E06E keyed at
# 25 s and restarted in P06 at 35 s. The typed rows send keys through the
# live DSKY's typed-key path (X86_TYPED, platform/x86/terminal_dsky.h: 20 cs
# apart, p holds PRO), in both the x86 run and the lockstep. The v69 rows
# key V69E at 23.5 s: the spin at VERB69 is checked against yaAGC's TC, and
# the TC trap's restart from 04000 on (its GOJAM is the x86 program's own:
# yaAGC's clock is frozen), so they must report the trap.
SCENARIOS = [('p00', 'lm', 4000, '', {}), ('p00', 'cm', 4000, '', {}),
             ('dap-hold', 'lm', 3000, '', fhd.DAP),
             ('v37e00e', 'cm', 3000, 'V37E00E', {}), ('v37e06e', 'cm', 3000, 'V37E06E', {}),
             ('p00-downrupt', 'lm', 4000, '', {'FLIGHT_DOWNRUPT': '1'}),
             ('p00-downrupt', 'cm', 4000, '', {'FLIGHT_DOWNRUPT': '1'}),
             ('restart', 'lm', 4000, '', {'FLIGHT_GOJAM': '2000', 'FLIGHT_DOWNRUPT': '1'}),
             ('v37e06e-restart', 'cm', 4000, '@2500V37E06E', {'FLIGHT_GOJAM': '3500', 'FLIGHT_DOWNRUPT': '1'}),
             ('typed-v35e', 'lm', 3000, '', {'X86_TYPED': 'v35e'}),
             ('typed-v37e00e-pro', 'lm', 3000, '', {'X86_TYPED': 'v37e00ep'}),
             ('typed-v37e06e-pro-marks', 'cm', 3000, '', {'X86_TYPED': 'v37e06epmj'}),
             ('v69', 'lm', 4000, '@2350V69E', {}), ('v69', 'cm', 4000, '@2350V69E', {}),
             ('v69-rset', 'lm', 4000, '@1000R@2350V69E@3000R', {}), ('v69-rset', 'cm', 4000, '@1000R@2350V69E@3000R', {})]
# The RESTART lamp each row must show (the x86 run's "RESTART lamp on/off"
# lines, in order). Power-up lights it, as yaAGC's first cycle does, so every
# row starts with "on"; v69-rset puts it out, lets the TC trap's GOJAM light
# it again, then puts it out again.
LAMP = {'v69-rset': ['on', 'off', 'on', 'off']}


def lockstep(cc, m, mode):
    """build/x86-flight/x86-flight-lockstep-<m>[-core]: the dispatcher of
    the x86 program, its scenario inputs and yaAGC's frozen-clock engine."""
    define, suffix = xb.MODES[mode]
    engine = fhd.OUT / 'agc_engine_scaled.o'
    if not engine.exists():
        fhd.frozen_engine(cc)
    flags = xb.flags(m, define) + [f'-I{fhd.YAAGC}']
    objs = []
    for src, path in (('flight_dispatch', 'platform/x86/flight_dispatch.c'), ('x86_flight_lockstep', 'tools/x86_flight_lockstep.c')):
        o = xb.OUT / f'{src}_{m}{suffix}_lockstep.o'
        subprocess.run([cc, *flags, '-c', path, '-o', str(o)], cwd=ROOT, check=True)
        objs.append(str(o))
    exe = xb.OUT / f'x86-flight-lockstep-{m}{suffix}'
    host = sorted(str(p) for p in (xb.STORE / 'host').glob('*.o'))
    subprocess.run([cc, *objs, *host, str(engine), str(fhd.YAAGC / 'agc_engine_init.o'), str(fhd.YAAGC / 'rfopen.o'),
                    '-Wl,--gc-sections', '-lm', '-o', str(exe)], cwd=ROOT, check=True)
    return exe


def lamp_test(cc, m):
    """build/x86-flight/test-restart-lamp-<m>: platform/x86/test_restart_lamp.c
    linked with mission m's dispatcher (x86_flight_build.build's object), so
    each of the lamp's light and clear paths is checked on its own."""
    flags = xb.flags(m, xb.MODES['compiled'][0])
    o = xb.OUT / f'test_restart_lamp_{m}.o'
    subprocess.run([cc, *flags, '-c', 'platform/x86/test_restart_lamp.c', '-o', str(o)], cwd=ROOT, check=True)
    exe = xb.OUT / f'test-restart-lamp-{m}'
    host = sorted(str(p) for p in (xb.STORE / 'host').glob('*.o'))
    subprocess.run([cc, str(o), str(xb.OUT / f'flight_dispatch_{m}.o'), *host, '-Wl,--gc-sections', '-lm',
                    '-o', str(exe)], cwd=ROOT, check=True)
    r = subprocess.run([str(exe)], cwd=ROOT, capture_output=True, text=True)
    print(f'{m}: {r.stdout.strip()}')
    return r.returncode


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('--host-cc', default=os.environ.get('CC', 'cc'))
    p.add_argument('--skip-index', action='store_true', help='reuse build/flight-store/<m>-host.inc and host objects')
    a = p.parse_args()
    if not a.skip_index:
        subprocess.run([sys.executable, 'tools/flight_store_build.py', '--host-only', '--host-cc', a.host_cc],
                       cwd=ROOT, check=True)
    CHECK.mkdir(parents=True, exist_ok=True)
    summary, failed = [], 0
    for mode in xb.MODES:
        exe = {m: xb.build(a.host_cc, m, mode) for m in ('lm', 'cm')}
        if mode == 'compiled':
            failed += sum(bool(lamp_test(a.host_cc, m)) for m in ('lm', 'cm'))
        ls = {m: lockstep(a.host_cc, m, mode) for m in ('lm', 'cm')}
        for name, m, cs, keys, env in SCENARIOS:
            tag = f'{m}-{name}-{mode}'
            run_env = {**os.environ, **env, 'X86_SPEED': '0', 'X86_REPORT': str(CHECK / f'{tag}-report.json')}
            args = [fhd.ROPE[m], str(cs)] + ([keys] if keys else [])
            r = subprocess.run([str(exe[m]), *args], cwd=ROOT, capture_output=True, text=True, env=run_env)
            k = subprocess.run([str(ls[m]), *args], cwd=ROOT, capture_output=True, text=True, env={**os.environ, **env})
            (CHECK / f'{tag}.log').write_text(r.stdout + r.stderr + '\n' + k.stdout + k.stderr)
            report = json.loads((CHECK / f'{tag}-report.json').read_text()) if (CHECK / f'{tag}-report.json').exists() else {}
            lamp = re.findall(r'^RESTART lamp (on|off) at', r.stdout, re.M)
            got = re.search(r'(\d+) routines and core slots checked .* (\d+) points yaAGC did not match', k.stdout)
            row = dict(scenario=tag, mission=m, interp=mode, centiseconds=cs, keys=keys, inputs=env,
                       run_exit=r.returncode, lockstep_exit=k.returncode,
                       checked=int(got[1]) if got else None, mismatched_points=int(got[2]) if got else None,
                       wall_seconds=report.get('wall_seconds'), core_slots=report.get('core_slots'),
                       core_points=[g['point'] for g in report.get('gaps', []) if g['kind'] == 'interp'],
                       unmapped=report.get('unmapped'), major_mode=report.get('major_mode'), restart_lamp=lamp)
            summary.append(row)
            bad = (r.returncode or k.returncode or (name.startswith('v69') and 'TC trap at' not in k.stdout)
                   or lamp != LAMP.get(name, ['on']))
            failed += bool(bad)
            print(f"{tag}: {row['checked']} routines checked, {row['mismatched_points']} points yaAGC did not match, "
                  f"{row['core_slots']} interpreter core slots at {len(row['core_points'])} points, "
                  f"{row['wall_seconds']} s flat out" + (' FAILED' if bad else ''))
    (CHECK / 'summary.json').write_text(json.dumps(summary, indent=1) + '\n')
    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main())
