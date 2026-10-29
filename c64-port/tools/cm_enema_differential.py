#!/usr/bin/env python3
"""CM ENEMA original LIGHTSET/cleanup/TVC/phase paths, full owned state."""
import hashlib,pathlib,subprocess
R=pathlib.Path(__file__).resolve().parents[1];F=R/'deps/cm-services-fixture';E=R/'deps/virtualagc/yaAGC'
assert (F/'FRESH_START_AND_RESTART.agc.original').read_bytes()==(R.parent/'Comanche055/FRESH_START_AND_RESTART.agc').read_bytes()
assert hashlib.sha256((F/'MAIN.agc.bin').read_bytes()).hexdigest()=='2ba31de9291cd10fb351a64d261bae8514a1cb75b4651bfa6a135dfa821a2d79'
subprocess.run(['cc','-std=c99','-O2','-DCM_RESTART_INPUT_ONLY','-I'+str(E),'-Iinclude','-Itests','tools/cm_enema_reference.c','tests/cm_restart_cases.c','tests/cm_enema_cases.c','tests/cm_executive_cases.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-enema-reference'],cwd=R,check=True)
for name,cmd in [('native',['./build/cm-enema-snapshot']),('reference',['./build/cm-enema-reference',str(F/'MAIN.agc.bin')])]:
 p=subprocess.run(cmd,cwd=R,text=True,capture_output=True,check=True)
 (R/f'build/cm-enema-{name}.snapshots').write_text(p.stdout);(R/f'build/cm-enema-{name}.trace').write_text(p.stderr)
a=(R/'build/cm-enema-native.snapshots').read_bytes();b=(R/'build/cm-enema-reference.snapshots').read_bytes();assert a==b
rows=[r.split() for r in a.decode().splitlines()];assert len(rows)==4096 and all(len(r)==441 for r in rows)
print('PASS:4096CMENEMA original checkpoints/all439owned words exact; sixteen guard words intact')
print('LIGHTSET main/nav resets, STARTSUB/STARTSB2 differences, TVCEXEC queue/WAIT aliases, rawA/L/Q/BBANK/SUPERBNK/IRQ and phase agreement')
print('Snapshot SHA256:',hashlib.sha256(b).hexdigest())
