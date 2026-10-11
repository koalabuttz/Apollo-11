#!/usr/bin/env python3
"""Native mixed-context continuations against unchanged original assembly."""
import hashlib
import pathlib
import re
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
reference = pathlib.Path(sys.argv[1]).resolve()
engine = reference.parent / 'virtualagc' / 'yaAGC'
for name in ('EXECUTIVE.agc', 'INTERPRETER.agc'):
    if (ROOT.parent / 'Luminary099' / name).read_bytes() != (reference / name).read_bytes():
        sys.exit('fixture source mismatch: ' + name)
listing = (reference / 'assembly.lst').read_text()
for name, address in [('CHANG1', '5122'), ('CHANG2', '5126'),
                      ('INTPRET', '6037'), ('INTRSM', '6050'),
                      ('DANZIG', '6061'), ('NEWOPS', '6070'), ('EXIT', '6743'),
                      ('JOBSLEEP', '5133'), ('JOBWAKE', '5137'),
                      ('PRIOCHNG', '5146'),
                      ('ENDJOB1', '01,3103'), ('NUDIRECT', '01,3225')]:
    if not re.search(r'(?<!\w)' + re.escape(name) + r'\s+' + re.escape(address) + r'\b', listing):
        sys.exit('fixture address mismatch: ' + name)
common = ['tests/scenarios.c', 'tests/interpreter_native_cases.c',
          'platform/host/interpreter_native_snapshot.c']
subprocess.run(['cc', '-std=c99', '-O2', '-Wall', '-Wextra', '-Werror',
    '-Wconversion', '-pedantic', '-Iinclude', '-Itests', 'src/agc_word.c',
    'src/executive.c', 'src/native_dispatch.c', 'src/irq_delivery.c', *common,
    '-o', 'build/interpreter-native-snapshot'], cwd=ROOT, check=True)
subprocess.run(['cc', '-std=c99', '-O2', '-DIN_INPUT_ONLY', '-DIN_ROW_ONLY',
    '-I' + str(engine), '-Iinclude', '-Itests',
    'tools/interpreter_native_reference.c', *common,
    str(engine / 'agc_engine.o'), str(engine / 'agc_engine_init.o'),
    str(engine / 'rfopen.o'), '-o', 'build/interpreter-native-reference'],
    cwd=ROOT, check=True)
outputs = []
for name, command in [('native', ['./build/interpreter-native-snapshot']),
        ('reference', ['./build/interpreter-native-reference', str(reference / 'MAIN.agc.bin')])]:
    run = subprocess.run(command, cwd=ROOT, capture_output=True, text=True)
    (ROOT / ('build/interpreter-native-' + name + '.snapshots')).write_text(run.stdout)
    if name == 'reference':
        (ROOT / 'build/interpreter-native-reference.trace').write_text(run.stderr)
    if run.returncode:
        sys.stderr.write(run.stderr)
        sys.exit(name + ' mixed-context runner failed')
    outputs.append([list(map(int, line.split())) for line in run.stdout.splitlines()])
if any(len(rows) != 140 for rows in outputs):
    sys.exit('expected 140 mixed-context checkpoints')
for row, (native, oracle) in enumerate(zip(*outputs)):
    if len(native) != 332 or len(oracle) != 332:
        sys.exit('expected 332 fields per mixed-context row')
    for field, (nv, rv) in enumerate(zip(native, oracle)):
        if nv != rv:
            sys.exit(f'case {native[0]}, step {native[1]}, field {field}: native={nv}, assembly={rv}')
print('PASS: 140 mixed-context original-assembly checkpoints; all 324 Executive words, status, continuation kind, address and interpreter control match')
print('Actual basic/interpretive transfers checked at original destinations with BBANK/SUPERBNK')
print('Snapshot SHA256: ' + hashlib.sha256((ROOT / 'build/interpreter-native-reference.snapshots').read_bytes()).hexdigest())
