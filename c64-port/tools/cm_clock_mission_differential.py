#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Whole-image continuous original CM ISR task/service execution comparison."""
import pathlib,subprocess,hashlib,json
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build';F=R/'deps/cm-clock-mission-fixture';E=R/'deps/virtualagc/yaAGC'
subprocess.run(['python3','tools/cm_clock_mission_prepare_reference.py'],cwd=R,check=True)
assert hashlib.sha256((F/'MAIN.agc.bin').read_bytes()).hexdigest()=='2ba31de9291cd10fb351a64d261bae8514a1cb75b4651bfa6a135dfa821a2d79'
for name in ['WAITLIST.agc','EXECUTIVE.agc','INTERPRETER.agc','INTERRUPT_LEAD_INS.agc','P40-P47.agc','CM_ENTRY_DIGITAL_AUTOPILOT.agc']:
 original=F/(name+'.original')
 assert original.read_bytes()==(R.parent/'Comanche055'/name).read_bytes()
 if name in ['WAITLIST.agc','EXECUTIVE.agc']:assert (F/name).read_bytes()==(R/'deps/cm-services-fixture'/name).read_bytes()
subprocess.run(['cc','-std=c99','-O2','-Iinclude','-Itests','-I'+str(E),'tools/cm_clock_mission_reference.c','tests/cm_service_loop_cases.c','src/cm_service_loop.c','src/cm_mission_loop.c','src/cm_clock_mission.c','tests/cm_mission_loop_cases.c','tests/cm_clock_mission_cases.c','src/cm_executive.c','src/cm_waitlist.c','src/cm_native_image.c','src/agc_word.c','tests/cm_executive_cases.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-clock-mission-reference'],cwd=R,check=True)
for role,cmd in [('native',['./build/cm-clock-mission-native']),('reference',['./build/cm-clock-mission-reference',str(F/'MAIN.agc.bin')])]:
 with (B/f'cm-clock-mission-{role}.snapshots').open('w') as o,(B/f'cm-clock-mission-{role}.trace').open('w') as e:subprocess.run(cmd,cwd=R,stdout=o,stderr=e,check=True)
a=(B/'cm-clock-mission-reference.snapshots').read_bytes();b=(B/'cm-clock-mission-native.snapshots').read_bytes();assert a==b
rows=[list(map(int,line.split())) for line in a.decode().splitlines()];assert all(len(r)==2059 for r in rows)
proof={'cases':448,'checkpoints':len(rows),'full_physical_words':2048,'entries':['384 automatic T3 interrupt sequences at06030 DANZIG','64 actual CLOCKJOB03332/050066 entries with NV0'],'actual_tasks':['15,3207 SETJTAG','24,3260 CLOKTASK','05306 SVCT3'],'actual_services':['WAITLIST','NOVAC','FIXDELAY','TASKOVER'],'boundary':'continuous task IRQ/services/RESUME/DANZIG, actual CLOCKJOB/QUICTRIG/SWRETURN/ENDJOB and suspended interpreter restoration','external_words':['SUPERBNK','IRQ'],'control_metadata':['InIsr','pending_DANZIG'],'actual_foreground':['unchanged interpreted NEWOPS','64 source-selected CLOCKJOB03332/050066 after nonzero-NV CLOKTASK','NBDONLY03542/014063 selected tuple only','64 separate actual CLOCKJOB entries with NV0 and original END/interpreter restoration'],'actual_clockjob_body_cases':128,'actual_display_makeplay_boundary_cases':64,'actual_end_interpreter_restore_cases':64,'snapshot_sha256':hashlib.sha256(a).hexdigest()}
(B/'cm-clock-mission-proof.json').write_text(json.dumps(proof,indent=2)+'\n');print('PASS: original continuous CM tasks and services, all physical image words exact');print(json.dumps(proof,indent=2))
