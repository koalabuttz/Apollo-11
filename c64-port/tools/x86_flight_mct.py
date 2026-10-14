#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Routine costs for the native x86_64 flight program's AGC clock
(docs/X86-FLIGHT.md): run the whole-program host differential's scenarios
(tools/flight_host_run.c, FLIGHT_MCT_OUT) and write, for every point a
routine matched yaAGC at, the median machine cycles yaAGC took for it, to
docs/flight-routine-mct.json: per mission, [interpretive, physical key,
cycles] sorted, and the median over all points of each kind as the default
for points no scenario reaches. One calibration for every AGC work clock:
the x86 program's (tools/x86_flight_build.py writes its table from it) and
the C64 resident's. Data only: no yaAGC code is linked from it.

  x86_flight_mct.py [--skip-build]   (after make flight-host-differential,
                                      or builds it)
"""
import argparse, json, os, statistics, subprocess, sys, tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MCT = ROOT / 'docs/flight-routine-mct.json'
sys.path.insert(0, str(ROOT / 'tools'))
import flight_host_differential as fhd  # noqa: E402

# (mission, centiseconds, DSKY keys, inputs): P00, the V37 programs the
# differential runs, and the LM DAP running (fhd.DAP).
SCENARIOS = [('lm', 4000, '', {}), ('cm', 4000, '', {}),
             ('lm', 3000, 'V37E00E', {}), ('cm', 3000, 'V37E00E', {}),
             ('lm', 3000, 'V37E12E', {}), ('cm', 3000, 'V37E06E', {}), ('lm', 3000, 'V37E63E', {}),
             ('lm', 3000, 'V37E12E', fhd.DAP)]


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('--host-cc', default=os.environ.get('CC', 'cc'))
    p.add_argument('--skip-build', action='store_true', help='reuse build/flight-store/flight-host-run-<m>')
    a = p.parse_args()
    if not a.skip_build:
        subprocess.run([sys.executable, 'tools/flight_store_build.py', '--host-only', '--host-cc', a.host_cc],
                       cwd=ROOT, check=True)
        engine = fhd.frozen_engine(a.host_cc)
        for m in ('lm', 'cm'):
            fhd.build(a.host_cc, m, engine)
    data = {}
    for m in ('lm', 'cm'):
        spans = {}
        with tempfile.TemporaryDirectory() as d:
            out = Path(d) / 'mct.txt'
            for mm, cs, keys, env in SCENARIOS:
                if mm != m:
                    continue
                subprocess.run([str(fhd.OUT / f'flight-host-run-{m}'), fhd.ROPE[m], str(cs)] + ([keys] if keys else []),
                               cwd=ROOT, check=True, stdout=subprocess.DEVNULL,
                               env={**os.environ, **env, 'FLIGHT_MCT_OUT': str(out)})
            for line in out.read_text().splitlines():
                interp, key, mct = map(int, line.split())
                spans.setdefault((interp, key), []).append(mct)
        cost = {k: max(1, min(65535, round(statistics.median(v)))) for k, v in spans.items()}
        kind = {i: [c for (j, _), c in cost.items() if j == i] for i in (0, 1)}
        data[m] = dict(runs=sum(len(v) for v in spans.values()),
                       default=dict(basic=round(statistics.median(kind[0])) if kind[0] else 20,
                                    interp=round(statistics.median(kind[1])) if kind[1] else 20),
                       points=[[i, k, c] for (i, k), c in sorted(cost.items())])
        print(f"{m}: {len(cost)} points, defaults {data[m]['default']}")
    # One point per line: [interpretive, key, cycles].
    MCT.write_text('{\n' + ',\n'.join(
        f' "{m}": {{"runs": {d["runs"]}, "default": {json.dumps(d["default"])}, "points": [\n'
        + ',\n'.join('  ' + json.dumps(x) for x in d['points']) + '\n ]}' for m, d in data.items()) + '\n}\n')
    print(f'wrote {MCT.relative_to(ROOT)}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
