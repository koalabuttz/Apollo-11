#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Compare canonical CHANG2 against the unchanged original source body."""
import argparse
import hashlib
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
p = argparse.ArgumentParser()
p.add_argument('--sanitize', action='store_true')
a = p.parse_args()
common = ['gcc', '-std=c99', '-O1' if a.sanitize else '-O2', '-Wall', '-Wextra', '-Werror', '-Iinclude', '-Itests']
if a.sanitize:
    common += ['-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-no-pie']
stem = 'lm-interpreter-change-sanitize' if a.sanitize else 'lm-interpreter-change-native'
subprocess.run(common + ['src/agc_word.c', 'src/lm_interpreter_change.c', 'tests/lm_interpreter_change_cases.c', 'tools/lm_interpreter_change_snapshot.c', '-o', 'build/' + stem], cwd=ROOT, check=True)
if not a.sanitize:
    for name in ('EXECUTIVE.agc', 'INTERPRETER.agc'):
        assert (ROOT.parent / 'Luminary099' / name).read_bytes() == (ROOT / 'deps/fixture' / name).read_bytes()
    subprocess.run(['python3', 'tools/prepare_hardware_dv.py'], cwd=ROOT, check=True)
    engine = ROOT / 'deps/virtualagc/yaAGC'
    subprocess.run(common + ['-I' + str(engine), 'tools/lm_interpreter_change_reference.c', 'tests/scenarios.c', 'src/agc_word.c', 'tests/lm_interpreter_change_cases.c', 'build/agc_engine_hardware_dv.o', str(engine / 'agc_engine_init.o'), str(engine / 'rfopen.o'), '-o', 'build/lm-interpreter-change-reference'], cwd=ROOT, check=True)
    with (ROOT / 'build/lm-interpreter-change-reference.snapshots').open('w') as out, (ROOT / 'build/lm-interpreter-change-reference.trace').open('w') as err:
        subprocess.run(['build/lm-interpreter-change-reference', 'deps/fixture/MAIN.agc.bin'], cwd=ROOT, stdout=out, stderr=err, check=True)
with (ROOT / ('build/' + stem + '.snapshots')).open('w') as out, (ROOT / ('build/' + stem + '.trace')).open('w') as err:
    subprocess.run(['build/' + stem], cwd=ROOT, stdout=out, stderr=err, check=True, env=dict(os.environ, ASAN_OPTIONS='detect_leaks=0'))
reference = ROOT / 'build/lm-interpreter-change-reference.snapshots'
native = ROOT / ('build/' + stem + '.snapshots')
errors = []
with reference.open() as left, native.open() as right:
    for expected, actual in zip(left, right):
        if expected != actual:
            x = list(map(int, expected.split()))
            y = list(map(int, actual.split()))
            errors.append((x[0], [(i - 2, a, b) for i, (a, b) in enumerate(zip(x, y)) if a != b]))
            if len(errors) == 8:
                break
if errors:
    raise SystemExit(str(errors))
assert reference.read_bytes() == native.read_bytes()
print('PASS: 16384 original-source CHANG2 transitions, all 2052 raw canonical/external words; basic and interpretive returns')
print('Snapshot SHA256:', hashlib.sha256(native.read_bytes()).hexdigest())
