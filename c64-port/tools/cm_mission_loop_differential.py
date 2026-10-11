#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Whole-image continuous original CM ISR task/service execution comparison."""
import pathlib,subprocess,hashlib,json
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build';F=R/'deps/cm-mission-fixture';E=R/'deps/virtualagc/yaAGC'
subprocess.run(['python3','tools/cm_mission_prepare_reference.py'],cwd=R,check=True)
assert hashlib.sha256((F/'MAIN.agc.bin').read_bytes()).hexdigest()=='2ba31de9291cd10fb351a64d261bae8514a1cb75b4651bfa6a135dfa821a2d79'
for name in ['WAITLIST.agc','EXECUTIVE.agc','INTERPRETER.agc','INTERRUPT_LEAD_INS.agc','P40-P47.agc','CM_ENTRY_DIGITAL_AUTOPILOT.agc']:
 original=F/(name+'.original')
 assert original.read_bytes()==(R.parent/'Comanche055'/name).read_bytes()
 if name in ['WAITLIST.agc','EXECUTIVE.agc']:assert (F/name).read_bytes()==(R/'deps/cm-services-fixture'/name).read_bytes()
subprocess.run(['cc','-std=c99','-O2','-Iinclude','-Itests','-I'+str(E),'tools/cm_mission_loop_reference.c','tests/cm_service_loop_cases.c','src/cm_service_loop.c','src/cm_mission_loop.c','tests/cm_mission_loop_cases.c','src/cm_executive.c','src/cm_waitlist.c','src/cm_native_image.c','src/agc_word.c','tests/cm_executive_cases.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-mission-loop-reference'],cwd=R,check=True)
for role,cmd in [('native',['./build/cm-mission-loop-native']),('reference',['./build/cm-mission-loop-reference',str(F/'MAIN.agc.bin')])]:
 with (B/f'cm-mission-loop-{role}.snapshots').open('w') as o,(B/f'cm-mission-loop-{role}.trace').open('w') as e:subprocess.run(cmd,cwd=R,stdout=o,stderr=e,check=True)
a=(B/'cm-mission-loop-reference.snapshots').read_bytes();b=(B/'cm-mission-loop-native.snapshots').read_bytes();assert a==b
rows=[list(map(int,line.split())) for line in a.decode().splitlines()];assert all(len(r)==2059 for r in rows)
proof={'cases':384,'checkpoints':len(rows),'full_physical_words':2048,'entries':['automatic T3 interrupt at06030 DANZIG'],'actual_tasks':['15,3207 SETJTAG','24,3260 CLOKTASK','05306 SVCT3'],'actual_services':['WAITLIST','NOVAC','FIXDELAY','TASKOVER'],'boundary':'continuous automatic IRQ through task ISR, real RESUME, pending DANZIG and actual selected foreground tuple','external_words':['SUPERBNK','IRQ'],'control_metadata':['InIsr','pending_DANZIG'],'actual_foreground':['unchanged interpreted NEWOPS','CLOCKJOB03332/050066','NBDONLY03542/014063'],'snapshot_sha256':hashlib.sha256(a).hexdigest()}
(B/'cm-mission-loop-proof.json').write_text(json.dumps(proof,indent=2)+'\n');print('PASS: original continuous CM tasks and services, all physical image words exact');print(json.dumps(proof,indent=2))
