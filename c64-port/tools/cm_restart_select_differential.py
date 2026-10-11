#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Full physical-image original Comanche restart selection comparison."""
import pathlib,subprocess,hashlib,json
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build';F=R/'deps/cm-restart-types-fixture';E=R/'deps/virtualagc/yaAGC'
B.mkdir(exist_ok=True)
subprocess.run(['python3','tools/cm_restart_types_prepare_reference.py'],cwd=R,check=True)
assert hashlib.sha256((F/'MAIN.agc.bin').read_bytes()).hexdigest()=='2ba31de9291cd10fb351a64d261bae8514a1cb75b4651bfa6a135dfa821a2d79'
assert (F/'RESTART_TABLES.agc.original').read_bytes()==(R.parent/'Comanche055/RESTART_TABLES.agc').read_bytes()
subprocess.run(['cc','-std=c99','-O2','-Iinclude','-Itests','-I'+str(E),'tools/cm_restart_select_reference.c','tests/cm_restart_select_cases.c','src/agc_word.c','tests/cm_executive_cases.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-restart-select-reference'],cwd=R,check=True)
for role,cmd in [('native',['./build/cm-restart-select-native']),('reference',['./build/cm-restart-select-reference',str(F/'MAIN.agc.bin')])]:
 with (B/f'cm-restart-select-{role}.snapshots').open('w') as o,(B/f'cm-restart-select-{role}.trace').open('w') as e:subprocess.run(cmd,cwd=R,stdout=o,stderr=e,check=True)
a=(B/'cm-restart-select-reference.snapshots').read_bytes();assert a==(B/'cm-restart-select-native.snapshots').read_bytes()
rows=[list(map(int,line.split())) for line in a.decode().splitlines()];assert len(rows)==6144 and all(len(row)==2050 for row in rows)
proof={'cases':6144,'full_physical_words':2048,'entries':['01,3520 RESTARTS','01,3755 PHSPART2','01,3533 GETPART2'],'boundary':'0704 GOLOC-1 or actual INITDSP FINDVAC service entry','snapshot_sha256':hashlib.sha256(a).hexdigest()}
(B/'cm-restart-select-proof.json').write_text(json.dumps(proof,indent=2)+'\n');print('PASS: 6144 original CM timing cases, every 2048 physical word and exit type exact');print(json.dumps(proof,indent=2))
