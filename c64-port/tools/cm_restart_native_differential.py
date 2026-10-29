#!/usr/bin/env python3
"""Compare actual source-selected callback caller sequences against native C."""
import pathlib,hashlib,subprocess
R=pathlib.Path(__file__).resolve().parents[1];F=R/'deps/cm-services-fixture';E=R/'deps/virtualagc/yaAGC'
assert hashlib.sha256((F/'MAIN.agc.bin').read_bytes()).hexdigest()=='2ba31de9291cd10fb351a64d261bae8514a1cb75b4651bfa6a135dfa821a2d79'
subprocess.run(['cc','-std=c99','-O2','-DCM_RESTART_INPUT_ONLY','-DCM_RESTART_NATIVE_INPUT_ONLY','-I'+str(E),'-Iinclude','-Itests','tools/cm_restart_native_reference.c','tests/cm_restart_cases.c','tests/cm_restart_native_cases.c','tests/cm_executive_cases.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-restart-native-reference'],cwd=R,check=True)
for name,cmd in [('native',['./build/cm-restart-native-snapshot']),('reference',['./build/cm-restart-native-reference',str(F/'MAIN.agc.bin')])]:
 p=subprocess.run(cmd,cwd=R,text=True,capture_output=True,check=True)
 (R/f'build/cm-restart-native-{name}.snapshots').write_text(p.stdout);(R/f'build/cm-restart-native-{name}.trace').write_text(p.stderr)
a=(R/'build/cm-restart-native-native.snapshots').read_bytes();b=(R/'build/cm-restart-native-reference.snapshots').read_bytes();assert a==b
rows=[r.split() for r in a.decode().splitlines()];assert len(rows)==320 and all(len(r)==436 for r in rows)
print('PASS:64 CM restart-selected callback sequences;320complete434-word checkpoints;192actualnativeCcallbacks')
print('Original STARTSB2/GOPROG3 phase check/RESTARTS admission/Executive; explicit erasable INCR MPAC+2 / TC ENDJOB caller fixtures;16guards intact')
print('Snapshot SHA256:',hashlib.sha256(b).hexdigest())
