#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""What an interpretive word costs the AGC: run flight scenarios on yaAGC
alone (tools/flight_host_run.c FLIGHT_WORD_OUT, the reference run) and
write, per mission, the interpretive words run and the machine cycles they
spent in the interpreter's banks (00 and 03, outside interrupts) to
docs/flight-interp-mct.json. The interpreter's work clock weight is the
mean per word:
- the C64 compact interpreter runs one word per call and charges it
  RK_INTERP_MCT (tools/resident_flight_build.py);
- the host's compiled interpretive routines are one per word, and a table
  with no calibrated runs in docs/flight-routine-mct.json gets this mean.
Time a word spends outside those banks (an RTB's routine, the basic code
after EXIT) is the other modules' and is charged where they run.

  interp_word_mct.py [--skip-build] [--keep DIR]   (DIR keeps the word lists)
"""
import argparse, json, os, subprocess, sys, tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'docs/flight-interp-mct.json'
sys.path.insert(0, str(ROOT / 'tools'))
import flight_host_differential as fhd  # noqa: E402

# The flight scenarios that run the most interpretive code (entry guidance;
# the descent's SERVICER and guidance with the engine on).
SCENARIOS = ['cm-entry', 'lm-p63-descent-thrust']


def words(path):
    """(centisecond, LOC, BANKSET, MCT in the interpreter, MCT elsewhere) per word."""
    return [tuple(int(x, 8) if i in (1, 2) else int(x) for i, x in enumerate(line.split()))
            for line in Path(path).read_text().splitlines()]


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('--host-cc', default=os.environ.get('CC', 'cc'))
    p.add_argument('--skip-build', action='store_true', help='reuse build/flight-store/flight-host-run-<m>')
    p.add_argument('--keep', help='directory to keep each scenario\'s word list in')
    a = p.parse_args()
    if not a.skip_build:
        subprocess.run([sys.executable, 'tools/flight_store_build.py', '--host-only', '--host-cc', a.host_cc],
                       cwd=ROOT, check=True)
        engine = fhd.frozen_engine(a.host_cc)
        for m in ('lm', 'cm'):
            fhd.build(a.host_cc, m, engine)
    data = {}
    with tempfile.TemporaryDirectory() as tmp:
        keep = Path(a.keep) if a.keep else Path(tmp)
        keep.mkdir(parents=True, exist_ok=True)
        for name, m, cs, keys, env in fhd.FLIGHT:
            if name not in SCENARIOS:
                continue
            out = keep / f'{name}.words'
            # The reference keeps 32,767 centiseconds.
            subprocess.run([str(fhd.OUT / f'flight-host-run-{m}'), fhd.ROPE[m], str(min(cs, 32767)), keys],
                           cwd=ROOT, check=True, stdout=subprocess.DEVNULL,
                           env={**os.environ, **env, 'FLIGHT_WORD_OUT': str(out)})
            w = words(out)
            d = data.setdefault(m, dict(scenarios=[], words=0, mct=0))
            d['scenarios'].append(name)
            d['words'] += len(w)
            d['mct'] += sum(x[3] for x in w)
    for m, d in data.items():
        d['mean'] = round(d['mct'] / d['words']) if d['words'] else 0
        print(f"{m}: {d['words']} words in {', '.join(d['scenarios'])}, {d['mean']} MCT a word")
    OUT.write_text(json.dumps(data, indent=1, sort_keys=True) + '\n')
    print(f'wrote {OUT.relative_to(ROOT)}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
