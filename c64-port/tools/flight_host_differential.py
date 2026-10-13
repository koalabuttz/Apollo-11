#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Whole-program host differential (docs/FLIGHT-STORE.md): build
tools/flight_host_run.c for both missions over every lane's routine tables
(tools/flight_store_build.py --host-only), run each scenario against yaAGC
and write the logs, and the unregistered or refused points by file and lane
(tools/flight_holes.py), to build/flight-store/differential/ (not
milestone evidence: phase 1 adds no certificate).

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
             ('lm', 3000, 'V37E12E'), ('cm', 3000, 'V37E06E')]


def build(cc, m):
    exe = OUT / f'flight-host-run-{m}'
    obj = OUT / f'flight_host_run_{m}.o'
    subprocess.run([cc, '-std=c99', '-O1', '-g', '-Wall', '-Wextra', '-Werror', '-Iinclude', '-Itests', f'-I{YAAGC}',
                    f'-DRK_{m.upper()}=1', f'-DFLIGHT_HOST_TABLES="../build/flight-store/{m}-host.inc"',
                    f'-DFLIGHT_HOST_SITES="../build/flight-store/{m}-sites.inc"',
                    '-c', 'tools/flight_host_run.c', '-o', str(obj)], cwd=ROOT, check=True)
    objs = sorted(str(p) for p in (OUT / 'host').glob('*.o'))
    subprocess.run([cc, str(obj), *objs, 'build/agc_engine_hardware_dv.o', str(YAAGC / 'agc_engine_init.o'),
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
    exe = {m: build(a.host_cc, m) for m in ('lm', 'cm')}
    EVIDENCE.mkdir(parents=True, exist_ok=True)
    words = {m: flight_holes.listing(flight_holes.MISSION[m]) for m in exe}
    lanes = flight_holes.lanes()
    summary, failed = [], 0
    for m, cs, keys in SCENARIOS:
        name = f'{m}-{keys or "p00-idle"}'
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
        row = dict(scenario=name, mission=m, centiseconds=cs, keys=keys, exit=r.returncode,
                   samples=int(s[1]) if s else 0, equal=int(s[2]) if s else 0, differ=int(s[3]) if s else None,
                   major_mode=dict(native=mode[1], yaagc=mode[2]) if mode else None,
                   routine_mismatches=[dict(point=x[0], table=x[1], times=int(x[2]), detail=x[3]) for x in misses],
                   unregistered=holes)
        summary.append(row)
        print(f"{name}: {row['equal']}/{row['samples']} samples equal, {len(row['routine_mismatches'])} routines "
              f"yaAGC did not match, {len(holes)} unregistered or refused points"
              + (f", major mode native {mode[1]} yaAGC {mode[2]}" if mode else '') + ('' if not r.returncode else ' FAILED'))
    (EVIDENCE / 'summary.json').write_text(json.dumps(summary, indent=1) + '\n')
    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main())
