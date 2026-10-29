#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma,hashlib,json,sys
R=pathlib.Path(__file__).resolve().parents[1];E=R/'deps/virtualagc/yaAGC'
core=['src/agc_multiword.c', 'src/agc_word.c', 'src/cm_admission_image.c', 'src/cm_basic_finish.c', 'src/cm_basic_priority.c', 'src/cm_basic_sleep.c', 'src/cm_basic_wake.c', 'src/cm_clock_display_builtin.c', 'src/cm_clock_display_mission.c', 'src/cm_clock_mission.c', 'src/cm_clock_mission_builtin.c', 'src/cm_clock_blocked_display.c', 'src/cm_display_rest.c', 'src/dsky_nv_admission.c', 'src/dsky_nv_busy.c', 'src/cm_clockjob.c', 'src/cm_clocktask.c', 'src/cm_danzig.c', 'src/cm_decimal_pair.c', 'src/cm_delay_image.c', 'src/cm_dispatch.c', 'src/cm_display_control.c', 'src/cm_entry_tasks.c', 'src/cm_executive.c', 'src/cm_formatter.c', 'src/cm_idle_dispatch.c', 'src/cm_init_display.c', 'src/cm_interbank.c', 'src/cm_longcall.c', 'src/cm_makeplay.c', 'src/cm_makeplay_sleep.c', 'src/cm_minutes_seconds.c', 'src/cm_mission_builtin.c', 'src/cm_mission_loop.c', 'src/cm_native_image.c', 'src/cm_normal_display.c', 'src/cm_normal_display_route.c', 'src/cm_nv85.c', 'src/cm_nv85_route.c', 'src/cm_phase.c', 'src/cm_pinball.c', 'src/cm_priority_complete.c', 'src/cm_quictrig.c', 'src/cm_restart_select.c', 'src/cm_restart_timing.c', 'src/cm_scale.c', 'src/cm_service_builtin.c', 'src/cm_service_loop.c', 'src/cm_svct3.c', 'src/cm_task_interrupt.c', 'src/cm_task_transfer.c', 'src/cm_tig5.c', 'src/cm_waitlist.c', 'src/cm_waitlist_image.c', 'src/dsky_decimal.c', 'src/dsky_pinball.c', 'src/dsky_scale.c', 'src/dsky_two_digit.c', 'src/fixed_data.c', 'src/guidance_product.c', 'tests/cm_clock_display_mission_cases.c', 'tests/cm_clock_mission_cases.c', 'tests/cm_clockjob_cases.c', 'tests/cm_mission_loop_cases.c', 'tests/cm_normal_display_cases.c', 'tests/cm_nv85_apply.c', 'tests/cm_nv85_cases.c', 'tests/cm_service_loop_cases.c', 'tests/cm_trig_cases.c']
flags=['cc','-O2','-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','-ffunction-sections','-Wl,--gc-sections']
subprocess.run(flags+core+['tools/cm_clock_blocked_display_snapshot.c','-o','build/cm-clock-blocked-display-native'],cwd=R,check=True)
subprocess.run([x for x in flags if x!='-Wconversion']+['-I'+str(E)]+core+['tools/cm_clock_blocked_display_reference.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-clock-blocked-display-reference'],cwd=R,check=True)
a=subprocess.Popen(['build/cm-clock-blocked-display-native'],cwd=R,stdout=subprocess.PIPE);b=subprocess.Popen(['build/cm-clock-blocked-display-reference','evidence/fixed-data-cm-original.rope.bin'],cwd=R,stdout=subprocess.PIPE);ah=hashlib.sha256();bh=hashlib.sha256();n=bad=0
with lzma.open(R/'build/cm-clock-blocked-display-native.snapshots.xz','wb',preset=3)as an,lzma.open(R/'build/cm-clock-blocked-display-reference.snapshots.xz','wb',preset=3)as bn:
 while True:
  x=a.stdout.readline();y=b.stdout.readline()
  if not x and not y:break
  if not x or not y:raise RuntimeError('length mismatch '+str(n))
  ah.update(x);bh.update(y);an.write(x);bn.write(y);n+=1
  if x!=y:
   if bad<20:
    av=list(map(int,x.split()));bv=list(map(int,y.split()));print('DIFF',av[:4],[(oct(i-4),oct(v),oct(w))for i,(v,w)in enumerate(zip(av,bv))if v!=w],flush=True)
   bad+=1
assert a.wait()==b.wait()==0
print(n,bad,ah.hexdigest(),bh.hexdigest(),flush=True)
(R/'build/cm-clock-blocked-display-stream-sha.json').write_text(json.dumps(dict(states=n,differing=bad,native=ah.hexdigest(),reference=bh.hexdigest()),indent=2)+'\n')
if bad:sys.exit(1)
