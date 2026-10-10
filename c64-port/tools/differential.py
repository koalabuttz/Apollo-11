#!/usr/bin/env python3
"""Compare native outputs with ORIGINAL Executive running in Virtual AGC."""
import argparse
import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
p = argparse.ArgumentParser()
p.add_argument('--reference', required=True, type=pathlib.Path,
               help='prepared fixture directory (contains MAIN.agc.bin)')
p.add_argument('--virtualagc', type=pathlib.Path,
               help='Virtual AGC checkout; default: fixture sibling virtualagc')
a = p.parse_args()
reference = a.reference.resolve()
virtual = (a.virtualagc or reference.parent / 'virtualagc').resolve()
engine = virtual / 'yaAGC'
original = ROOT.parent / 'Luminary099/EXECUTIVE.agc'
if original.read_bytes() != (reference / 'EXECUTIVE.agc').read_bytes():
    sys.exit('FAIL: fixture Executive is not byte-identical to upstream')
subprocess.run(['gcc', '-std=c99', '-O2', '-I'+str(engine), '-Iinclude', '-Itests',
                'tools/reference_runner.c', 'tests/scenarios.c',
                str(engine/'agc_engine.o'), str(engine/'agc_engine_init.o'),
                str(engine/'rfopen.o'), '-o', 'build/reference-runner'], cwd=ROOT, check=True)
native = subprocess.run(['./build/snapshot'], cwd=ROOT, capture_output=True, text=True, check=True)
oracle = subprocess.run(['./build/reference-runner', str(reference/'MAIN.agc.bin')],
                        cwd=ROOT, capture_output=True, text=True, check=True)
(ROOT/'build/native.snapshots').write_text(native.stdout)
(ROOT/'build/reference.snapshots').write_text(oracle.stdout)
(ROOT/'build/reference.trace').write_text(oracle.stderr)
nr, rr = native.stdout.splitlines(), oracle.stdout.splitlines()
if len(nr) != 26 or len(rr) != 26:
    sys.exit('FAIL: expected exactly 26 output scenarios')
errors=[]
for n,(native_row,reference_row) in enumerate(zip(nr,rr)):
    for k,(nv,rv) in enumerate(zip(native_row.split(),reference_row.split())):
        if nv != rv:
            errors.append(f'scenario {n}, field {k}: native={nv}, reference={rv}')
    if len(native_row.split()) != len(reference_row.split()):
        errors.append(f'scenario {n}: row length mismatch')
if errors:
    print('\n'.join(errors)); sys.exit(1)
print('PASS: 26 original-assembly scenarios; 323 AGC state words plus alarm and status per scenario agree')
print('Boundaries: ENDFIND / BAILOUT1 / CHANJOB-2 / DUMMYJOB entry; no restart or dispatch claim')
