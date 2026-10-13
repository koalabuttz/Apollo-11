#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Run several Lane C C64 programs in VICE and total their checks.

  python3 tools/lanec/vice_many.py NAME PRG...

Runs tools/vice_run.py on each PRG in turn (its own PASS line is printed),
stops at the first failure, and ends with one line for NAME:
"PASS: VICE PAL NAME: N programs, M checks, 0 failures".
"""
import pathlib, re, subprocess, sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
name, prgs = sys.argv[1], sys.argv[2:]
checks = 0
for prg in prgs:
    run = subprocess.run([sys.executable, str(ROOT / 'tools/vice_run.py'), '--cycles', '900000000', prg],
                         cwd=ROOT, capture_output=True, text=True)
    out = (run.stdout + run.stderr).strip().splitlines()
    line = [x for x in out if x.startswith(('PASS', 'FAIL'))][-1:] or out[-1:] or ['no output']
    print(line[0])
    m = re.match(r'PASS: VICE PAL \S+: (\d+) checks, 0 failures', line[0])
    if run.returncode or not m:
        sys.exit(f'FAIL: VICE PAL {name}: {prg}')
    checks += int(m.group(1))
print(f'PASS: VICE PAL {name}: {len(prgs)} programs, {checks} checks, 0 failures')
