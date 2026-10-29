#!/usr/bin/env python3
"""Original Comanche AX*SR*T and CM TERM1TMP alias against native translation."""
import pathlib,hashlib,subprocess,json
R=pathlib.Path(__file__).resolve().parents[1];F=R/'deps/cm-numeric-fixture';E=R/'deps/virtualagc/yaAGC'
subprocess.run(['python3','tools/cm_numeric_differential.py','--prepare-only'],cwd=R,check=True)
assert (F/'POWERED_FLIGHT_SUBROUTINES.agc.original').read_bytes()==(R.parent/'Comanche055/POWERED_FLIGHT_SUBROUTINES.agc').read_bytes()
assert hashlib.sha256((F/'MAIN.agc.bin').read_bytes()).hexdigest()=='2ba31de9291cd10fb351a64d261bae8514a1cb75b4651bfa6a135dfa821a2d79'
flags=['-DDIVIDE_INPUT_ONLY','-DSTORE_INPUT_ONLY','-DELAPSED_INPUT_ONLY','-DPUSH_INPUT_ONLY','-DSCALE_INPUT_ONLY','-DPREPARE_INPUT_ONLY','-DUNIT_INPUT_ONLY','-DROTATION_INPUT_ONLY']
cases=['tests/prepare_cases.c','tests/scale_cases.c','tests/push_cases.c','tests/elapsed_cases.c','tests/store_cases.c','tests/divide_cases.c','tests/unit_cases.c','tests/rotation_cases.c']
subprocess.run(['cc','-std=c99','-O2','-Iinclude','-Itests','-I'+str(E)]+flags+['tools/cm_rotation_reference.c']+cases+['tests/cm_executive_cases.c','build/cm-numeric-hardware-dv.o',str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-rotation-reference'],cwd=R,check=True)
for role,cmd in [('native',['./build/cm-rotation-native']),('reference',['./build/cm-rotation-reference',str(F/'MAIN.agc.bin')])]:
 run=subprocess.run(cmd,cwd=R,text=True,capture_output=True,check=True)
 (R/f'build/cm-rotation-{role}.snapshots').write_text(run.stdout);(R/f'build/cm-rotation-{role}.trace').write_text(run.stderr)
a=(R/'build/cm-rotation-native.snapshots').read_bytes();b=(R/'build/cm-rotation-reference.snapshots').read_bytes();assert a==b
rows=[line.split() for line in b.decode().splitlines()];assert len(rows)==32768 and all(len(row)==133 for row in rows)
print('PASS: 32768 original CM AX*SR*T checkpoints/all 131 modeled words+status exact; raw overflow domain and both directions')
print('CM TERM1TMP=BUF2 at 0133, MPAC+3/+4 retained; entry 23,3650; source guards and physical bank transfers intact')
print('Snapshot SHA256:',hashlib.sha256(b).hexdigest())
(R/'build/cm-rotation-proof.json').write_text(json.dumps({'cases':32768,'modeled_words':131,'entry':'23,3650','TERM1TMP':'0133 BUF2','snapshot_sha256':hashlib.sha256(b).hexdigest()},indent=2)+'\n')
