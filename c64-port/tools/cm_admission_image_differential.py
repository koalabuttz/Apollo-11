#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Full physical-image original Comanche admission comparison."""
import pathlib,subprocess,hashlib,json
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build';F=R/'deps/cm-services-fixture';E=R/'deps/virtualagc/yaAGC'
B.mkdir(exist_ok=True)
if not (F/'MAIN.agc.bin').exists():subprocess.run(['python3','tools/cm_services_prepare_reference.py'],cwd=R,check=True)
assert hashlib.sha256((F/'MAIN.agc.bin').read_bytes()).hexdigest()=='2ba31de9291cd10fb351a64d261bae8514a1cb75b4651bfa6a135dfa821a2d79'
assert (F/'EXECUTIVE.agc.original').read_bytes()==(R.parent/'Comanche055/EXECUTIVE.agc').read_bytes()
subprocess.run(['cc','-std=c99','-O2','-Iinclude','-Itests','-I'+str(E),'tools/cm_admission_image_reference.c','tests/cm_admission_image_cases.c','src/agc_word.c','src/cm_executive.c','src/cm_native_image.c','tests/cm_executive_cases.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-admission-image-reference'],cwd=R,check=True)
for role,cmd in [('native',['./build/cm-admission-image-native']),('reference',['./build/cm-admission-image-reference',str(F/'MAIN.agc.bin')])]:
 with (B/f'cm-admission-image-{role}.snapshots').open('w') as o,(B/f'cm-admission-image-{role}.trace').open('w') as e:subprocess.run(cmd,cwd=R,stdout=o,stderr=e,check=True)
a=(B/'cm-admission-image-reference.snapshots').read_bytes();assert a==(B/'cm-admission-image-native.snapshots').read_bytes()
rows=[list(map(int,line.split())) for line in a.decode().splitlines()];assert len(rows)==1024 and all(len(row)==2052 for row in rows)
proof={'cases':1024,'full_physical_words':2048,'entries':['05147 FINDVAC','05134 NOVAC'],'boundary':'actual source Q+2 return','external_words':['SUPERBNK','IRQ'],'snapshot_sha256':hashlib.sha256(a).hexdigest()}
(B/'cm-admission-image-proof.json').write_text(json.dumps(proof,indent=2)+'\n');print('PASS: 1024 original CM timing cases, every 2048 physical word and exit type exact');print(json.dumps(proof,indent=2))
