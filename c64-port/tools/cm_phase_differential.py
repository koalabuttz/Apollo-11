#!/usr/bin/env python3
"""Exhaust CM phase words at both source entries with exact full states."""
import hashlib,pathlib,subprocess
R=pathlib.Path(__file__).resolve().parents[1];F=R/'deps/cm-services-fixture';E=R/'deps/virtualagc/yaAGC'
subprocess.run(['python3','tools/cm_services_prepare_reference.py'],cwd=R,check=True)
assert (R.parent/'Comanche055/PHASE_TABLE_MAINTENANCE.agc').read_bytes()==(F/'PHASE_TABLE_MAINTENANCE.agc').read_bytes()
subprocess.run(['cc','-std=c99','-O2','-I'+str(E),'-Iinclude','-Itests','tools/cm_phase_reference.c','tests/cm_executive_cases.c','tests/cm_phase_cases.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-phase-reference'],cwd=R,check=True)
for name,cmd in [('native',['./build/cm-phase-snapshot']),('reference',['./build/cm-phase-reference',str(F/'MAIN.agc.bin')])]:
 p=subprocess.run(cmd,cwd=R,text=True,capture_output=True,check=True)
 (R/f'build/cm-phase-{name}.snapshots').write_text(p.stdout)
 (R/f'build/cm-phase-{name}.trace').write_text(p.stderr)
a=(R/'build/cm-phase-native.snapshots').read_bytes();b=(R/'build/cm-phase-reference.snapshots').read_bytes();assert a==b
rows=[r.split() for r in a.decode().splitlines()];assert len(rows)==49152 and all(len(r)==58 for r in rows)
print('PASS: CM PHASCHNG/2PHSCHNG all 49,152 valid phase words/groups; all56state/register words exact; guarded memory and parameter streams intact')
print('Snapshot SHA256:',hashlib.sha256(b).hexdigest())
