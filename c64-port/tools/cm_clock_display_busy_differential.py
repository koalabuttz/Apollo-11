#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,hashlib,json
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build';E=R/'deps/virtualagc/yaAGC';F=R/'evidence/fixed-data-cm-original.rope.bin'
assert hashlib.sha256(F.read_bytes()).hexdigest()=='6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061'
assert (B/'fixed-data/Comanche055/MAIN.agc.bin').read_bytes()==F.read_bytes()
subprocess.run(['cc','-std=c99','-O2','-Iinclude','-Itests','-I'+str(E),'tools/cm_clock_display_busy_reference.c','tests/cm_service_loop_cases.c','src/cm_service_loop.c','src/cm_mission_loop.c','src/cm_clock_mission.c','src/cm_clock_display_mission.c','src/cm_clock_display_builtin.c','tests/cm_clock_display_mission_cases.c','tests/cm_clock_display_busy_cases.c','tests/cm_mission_loop_cases.c','tests/cm_clock_mission_cases.c','src/cm_executive.c','src/cm_waitlist.c','src/cm_native_image.c','src/agc_word.c','tests/cm_executive_cases.c', 'src/cm_priority_complete.c','src/cm_dispatch.c','src/cm_makeplay.c','src/cm_makeplay_sleep.c','src/cm_basic_wake.c','src/cm_basic_sleep.c','src/cm_clock_mission_builtin.c','src/cm_clockjob.c','src/cm_quictrig.c','src/cm_display_control.c','src/cm_idle_dispatch.c','src/cm_mission_builtin.c','src/cm_service_builtin.c','src/cm_task_interrupt.c','src/cm_task_transfer.c','src/cm_entry_tasks.c','src/cm_clocktask.c','src/cm_tig5.c','src/cm_svct3.c','src/cm_waitlist_image.c','src/cm_admission_image.c','src/cm_delay_image.c','src/cm_longcall.c','src/cm_restart_select.c','src/cm_restart_timing.c','src/cm_init_display.c','src/cm_basic_priority.c','src/cm_phase.c','src/agc_multiword.c','src/cm_danzig.c','src/cm_basic_finish.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-clock-display-busy-reference'],cwd=R,check=True)

for role,cmd in [('native',['./build/cm-clock-display-busy-native']),('reference',['./build/cm-clock-display-busy-reference',str(F)])]:
 with (B/f'cm-clock-display-busy-{role}.snapshots').open('w') as o,(B/f'cm-clock-display-busy-{role}.trace').open('w') as e:subprocess.run(cmd,cwd=R,stdout=o,stderr=e,check=True)
a=(B/'cm-clock-display-busy-reference.snapshots').read_bytes();b=(B/'cm-clock-display-busy-native.snapshots').read_bytes()
assert a==b
rows=[list(map(int,line.split())) for line in a.decode().splitlines()];assert all(len(r)==2060 for r in rows)
assert all([r for r in rows if r[0]==n][-1][9]==0o6037 for n in range(64))
assert sum(r[2]==103 for r in rows)==64
proof=dict(cases=64,checkpoints=len(rows),full_physical_words=2048,source_rope_sha256=hashlib.sha256(F.read_bytes()).hexdigest(),snapshot_sha256=hashlib.sha256(a).hexdigest(),scope='actual T3/CLOKTASK-admitted CLOCKJOB/QUICTRIG/MAKEPLAY/PRIOCHNG with matching busy TEMPFLSH051366/DSPLOCK1..7 through genuine END and restored interpreter; no noun40 conversion',groups=sorted(set(x[2] for x in rows)))
(B/'cm-clock-display-busy-proof.json').write_text(json.dumps(proof,indent=2)+'\n');print('PASS: actual uninterrupted CM clock display source groups, all physical words and channel11 exact');print(json.dumps(proof,indent=2))
