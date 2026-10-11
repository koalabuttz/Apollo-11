#!/usr/bin/env python3
"""CM cleanup/startsub/phase/job boundaries against original assembly."""
import hashlib,pathlib,subprocess
R=pathlib.Path(__file__).resolve().parents[1];F=R/'deps/cm-services-fixture';E=R/'deps/virtualagc/yaAGC'
assert (F/'RESTARTS_ROUTINE.agc').read_bytes()==(R.parent/'Comanche055/RESTARTS_ROUTINE.agc').read_bytes()
assert (F/'FRESH_START_AND_RESTART.agc.original').read_bytes()==(R.parent/'Comanche055/FRESH_START_AND_RESTART.agc').read_bytes()
assert hashlib.sha256((F/'MAIN.agc.bin').read_bytes()).hexdigest()=='2ba31de9291cd10fb351a64d261bae8514a1cb75b4651bfa6a135dfa821a2d79'
subprocess.run(['cc','-std=c99','-O2','-DCM_RESTART_INPUT_ONLY','-I'+str(E),'-Iinclude','-Itests','tools/cm_restart_reference.c','tests/cm_restart_cases.c','tests/cm_executive_cases.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-restart-reference'],cwd=R,check=True)
for name,cmd in [('native',['./build/cm-restart-snapshot']),('reference',['./build/cm-restart-reference',str(F/'MAIN.agc.bin')])]:
 p=subprocess.run(cmd,cwd=R,text=True,capture_output=True,check=True)
 (R/f'build/cm-restart-{name}.snapshots').write_text(p.stdout);(R/f'build/cm-restart-{name}.trace').write_text(p.stderr)
a=(R/'build/cm-restart-native.snapshots').read_bytes();b=(R/'build/cm-restart-reference.snapshots').read_bytes();assert a==b
rows=[r.split() for r in a.decode().splitlines()];assert len(rows)==1280 and all(len(r)==431 for r in rows)
print('PASS: 1280 CM original source checkpoints; all429owned words+status exact; sixteen guard words intact')
print('Boundaries: STARTSB2/STARTSUB return, GOPROG3 beforeMMDSPLAY/PTBAD, RESTARTS variablejob throughENDFIND/BAILOUT')
print('Snapshot SHA256:',hashlib.sha256(b).hexdigest())
