#!/usr/bin/env python3
"""Original CM startup/cleanup across every image word except explicit Z."""
import pathlib,hashlib,subprocess,json
R=pathlib.Path(__file__).resolve().parents[1];F=R/'deps/cm-services-fixture';E=R/'deps/virtualagc/yaAGC'
assert (F/'FRESH_START_AND_RESTART.agc.original').read_bytes()==(R.parent/'Comanche055/FRESH_START_AND_RESTART.agc').read_bytes()
assert hashlib.sha256((F/'MAIN.agc.bin').read_bytes()).hexdigest()=='2ba31de9291cd10fb351a64d261bae8514a1cb75b4651bfa6a135dfa821a2d79'
subprocess.run(['cc','-std=c99','-O2','-I'+str(E),'-Iinclude','-Itests','tools/cm_startup_reference.c','tests/cm_startup_cases.c','tests/cm_executive_cases.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-startup-reference'],cwd=R,check=True)
for name,cmd in [('native',['./build/cm-startup-snapshot']),('reference',['./build/cm-startup-reference',str(F/'MAIN.agc.bin')])]:
 p=subprocess.run(cmd,cwd=R,text=True,capture_output=True,check=True)
 (R/f'build/cm-startup-{name}.snapshots').write_text(p.stdout);(R/f'build/cm-startup-{name}.trace').write_text(p.stderr)
a=(R/'build/cm-startup-native.snapshots').read_bytes();b=(R/'build/cm-startup-reference.snapshots').read_bytes();assert a==b
rows=[r.split() for r in a.decode().splitlines()];assert len(rows)==640 and all(len(r)==2064 for r in rows)
print('PASS:640 CM original startup/cleanup checkpoints; every2047image word exceptZ +15externalI/O values exact')
print('SLAP1/DOFSTART throughENDRSTRT; MR.KLEAN/P00KLEAN/V37KLEAN throughTC Q; rawA/L andbank/IRQ included')
print('Peripheral drive counters held atzero and scaler frozen; no original flightword normalization; Z excluded because native hasnoAGCinstruction counter')
print('Snapshot SHA256:',hashlib.sha256(b).hexdigest())
(R/'build/cm-startup-proof.json').write_text(json.dumps({'cases':640,'image_words':2047,'external_io_words':15,'row_values_with_status':2063,'excluded_source_addresses':['0005 Z'],'hardware_drive_counters_held_zero':['0035','0036','0047','0050','0051','0052','0053','0054'],'snapshot_sha256':hashlib.sha256(b).hexdigest()},indent=2)+'\n')
