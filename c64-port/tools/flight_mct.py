#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Calibrate the flight resident's work clock (docs/RESIDENT-FLIGHT.md): the
yaAGC machine cycles (MCT) each native routine took, by routine table, from
the host flight run's lockstep (tools/flight_host_run.c FLIGHT_MCT) over the
reference scenarios: the flight host differential's, plus Lane F's DAP hold
and firing. Writes docs/flight-mct.json, which tools/flight_store_build.py
turns into one MCT weight per flight module (MCT per routine run) and
tools/resident_flight_build.py into the resident tables' weight.

Every routine yaAGC matched counts; one key's runs give n, sum, sum of
squares, least and most, so a module's weight (its tables' sum / n) and its
residual (RMS and worst run against that weight) follow exactly.

  flight_mct.py [--host-cc CC] [--skip-index] [--jobs N]
"""
import argparse
import collections
import json
import os
import pathlib
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import flight_host_differential as fhd  # noqa: E402

OUT = ROOT / 'build/flight-store/mct'
DATA = ROOT / 'docs/flight-mct.json'
DAP = {'FLIGHT_CH30': '036377', 'FLIGHT_CH31': '057777'}
SCENARIOS = [(m, cs, keys, {}) for m, cs, keys in fhd.SCENARIOS] + [
    ('lm', 3200, '', DAP),
    ('lm', 3200, '', dict(DAP, FLIGHT_POKE='3000:033=02000,3000:034=077000')),
    ('lm', 3000, 'V37E12E', fhd.DAP)]


def run(exe, m, cs, keys, env, i):
    out = OUT / f'{i:02d}-{m}.txt'
    r = subprocess.run([str(exe[m]), fhd.ROPE[m], str(cs)] + ([keys] if keys else []), cwd=ROOT,
                       capture_output=True, text=True, env={**os.environ, **env, 'FLIGHT_MCT': str(out)})
    (OUT / f'{i:02d}-{m}.log').write_text(r.stdout + r.stderr)
    if not out.exists():
        sys.exit(f'FAIL: scenario {i} ({m} {cs} {keys} {env}) wrote no MCT table')
    return m, out


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('--host-cc', default=os.environ.get('CC', 'cc'))
    p.add_argument('--skip-index', action='store_true', help='reuse build/flight-store/<m>-host.inc and host objects')
    p.add_argument('--jobs', type=int, default=os.cpu_count() or 4)
    a = p.parse_args()
    if not a.skip_index:
        subprocess.run([sys.executable, 'tools/flight_store_build.py', '--host-only', '--host-cc', a.host_cc],
                       cwd=ROOT, check=True)
    OUT.mkdir(parents=True, exist_ok=True)
    engine = fhd.frozen_engine(a.host_cc)
    exe = {m: fhd.build(a.host_cc, m, engine) for m in ('lm', 'cm')}
    keys = {m: collections.defaultdict(lambda: [0, 0, 0, None, 0]) for m in exe}
    with ThreadPoolExecutor(a.jobs) as pool:
        done = list(pool.map(lambda x: run(exe, *x[1], x[0]), enumerate(SCENARIOS)))
    for m, out in done:
        for line in out.read_text().splitlines():
            table, key, interp, n, total, sq, least, most = line.split()
            k = keys[m][f'{table} {interp} {key}']
            k[0] += int(n)
            k[1] += int(total)
            k[2] += int(float(sq))
            k[3] = int(least) if k[3] is None else min(k[3], int(least))
            k[4] = max(k[4], int(most))
    data = dict(scenarios=[dict(mission=m, centiseconds=cs, keys=k, env=env) for m, cs, k, env in SCENARIOS],
                missions={m: {k: dict(n=v[0], sum=v[1], sq=v[2], least=v[3], most=v[4]) for k, v in sorted(ks.items())}
                          for m, ks in keys.items()})
    DATA.write_text(json.dumps(data, indent=0, sort_keys=True) + '\n')
    for m, ks in keys.items():
        n = sum(v[0] for v in ks.values())
        print(f'{m}: {len(ks)} entry keys, {n} routine runs, {sum(v[1] for v in ks.values()) / max(n, 1):.1f} MCT per run')
    return 0


if __name__ == '__main__':
    sys.exit(main())
