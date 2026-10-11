#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Full physical-image original Comanche CLOKTASK comparison."""
import pathlib,subprocess,hashlib,json
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build';F=R/'deps/cm-entry-tasks-fixture';E=R/'deps/virtualagc/yaAGC'
B.mkdir(exist_ok=True)
subprocess.run(['python3','tools/cm_entry_tasks_prepare_reference.py'],cwd=R,check=True)
assert hashlib.sha256((F/'MAIN.agc.bin').read_bytes()).hexdigest()=='2ba31de9291cd10fb351a64d261bae8514a1cb75b4651bfa6a135dfa821a2d79'
assert (F/'P40-P47.agc.original').read_bytes()==(R.parent/'Comanche055/P40-P47.agc').read_bytes()
subprocess.run(['cc','-std=c99','-O2','-Iinclude','-Itests','-I'+str(E),'tools/cm_clocktask_reference.c','tests/cm_clocktask_cases.c','src/agc_word.c','tests/cm_executive_cases.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-clocktask-reference'],cwd=R,check=True)
for role,cmd in [('native',['./build/cm-clocktask-native']),('reference',['./build/cm-clocktask-reference',str(F/'MAIN.agc.bin')])]:
 with (B/f'cm-clocktask-{role}.snapshots').open('w') as o,(B/f'cm-clocktask-{role}.trace').open('w') as e:subprocess.run(cmd,cwd=R,stdout=o,stderr=e,check=True)
a=(B/'cm-clocktask-reference.snapshots').read_bytes();assert a==(B/'cm-clocktask-native.snapshots').read_bytes()
rows=[list(map(int,line.split())) for line in a.decode().splitlines()];assert len(rows)==1536 and all(len(row)==2052 for row in rows)
proof={'cases':1536,'full_physical_words':2048,'entries':['24,3260 CLOKTASK','24,3301 after WAITLIST','24,3315 SETTB6'],'boundary':'actual WAITLIST/NOVAC/FINDVAC/TASKOVER entry','external_words':['SUPERBNK','IRQ'],'snapshot_sha256':hashlib.sha256(a).hexdigest()}
(B/'cm-clocktask-proof.json').write_text(json.dumps(proof,indent=2)+'\n');print('PASS: 1536 original CM timing cases, every 2048 physical word and exit type exact');print(json.dumps(proof,indent=2))
