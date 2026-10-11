#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Live Executive through actual returned UNIT/radial/acceleration/projection phases."""
import argparse,pathlib,subprocess,hashlib
root=pathlib.Path(__file__).resolve().parents[1];p=argparse.ArgumentParser();p.add_argument('--sanitize',action='store_true');a=p.parse_args()
inputs=[f'tests/{name}_cases.c'for name in ('prepare','scale','push','elapsed','store','divide','gravity','velocity','radial','unit','trig','projection','p66_guidance_live')]
defines=['-D'+name+'_INPUT_ONLY'for name in ('PREPARE','SCALE','PUSH','ELAPSED','STORE','DIVIDE','GRAVITY','VELOCITY','RADIAL','UNIT','TRIG','PROJECTION')]
numerical=['src/guidance.c']+[f'src/guidance_{name}.c'for name in ('velocity_phased','vector','control_observer','prepare','scale','push','elapsed','store','divide','gravity','product','shift','velocity','dp_divide','norm','unit','unit_phased','dot','sum','radial','radial_phased','acceleration','acceleration_phased','acceleration_store','trig','rotation','projection','projection_phased')]
base=['gcc','-std=c99','-O1'if a.sanitize else '-O2','-Wall','-Wextra','-Werror','-ffunction-sections','-Wl,--gc-sections','-Iinclude','-Itests']
if a.sanitize:base+=['-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']
name='p66-guidance-live-sanitize'if a.sanitize else 'p66-guidance-live-native'
subprocess.run(base+defines+['platform/host/p66_guidance_live_snapshot.c','src/agc_word.c','src/agc_multiword.c','src/executive.c','src/native_dispatch.c','src/irq_delivery.c','src/p66_native.c','src/p66_native_guidance.c','src/p66_guidance_invoke.c']+numerical+inputs+['-o','build/'+name],cwd=root,check=True)
subprocess.run(base+defines+['tools/p66_guidance_live_inputs.c','src/agc_word.c','src/agc_multiword.c','src/executive.c','src/p66_native.c','-DP66_NATIVE_DISPATCH_ONLY']+inputs+['-o','build/p66-guidance-live-inputs'],cwd=root,check=True)
initial=subprocess.check_output(['build/p66-guidance-live-inputs'],cwd=root,text=True);(root/'build/p66-guidance-live-inputs.snapshots').write_text(initial)
if not a.sanitize:
 for source in ('EXECUTIVE.agc','INTERPRETER.agc','LUNAR_LANDING_GUIDANCE_EQUATIONS.agc'):
  assert (root.parent/'Luminary099'/source).read_bytes()==(root/'deps/fixture'/source).read_bytes(),source
 subprocess.run(['python3','tools/prepare_hardware_dv.py'],cwd=root,check=True)
 engine=root/'deps/virtualagc/yaAGC'
 subprocess.run(base+defines+['-DP66_NATIVE_DISPATCH_ONLY','-I'+str(engine),'tools/p66_guidance_live_reference.c','tests/scenarios.c','src/agc_word.c','src/agc_multiword.c','src/executive.c','src/p66_native.c']+inputs+['build/agc_engine_hardware_dv.o',str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/p66-guidance-live-reference'],cwd=root,check=True)
 run=subprocess.run(['build/p66-guidance-live-reference','deps/fixture/MAIN.agc.bin'],capture_output=True,text=True,cwd=root,check=True);(root/'build/p66-guidance-live-reference.snapshots').write_text(run.stdout);(root/'build/p66-guidance-live-reference.trace').write_text(run.stderr)
run=subprocess.run(['build/'+name],capture_output=True,text=True,cwd=root,check=True);(root/'build'/f'{name}.snapshots').write_text(run.stdout)
got=[list(map(int,l.split()))for l in run.stdout.splitlines()];expected=[list(map(int,l.split()))for l in (root/'build/p66-guidance-live-reference.snapshots').read_text().splitlines()]
assert len(got)==len(expected)==2944 and all(len(r)==495 for r in got)
errors=[(row[:2],[(i,x,y)for i,(x,y)in enumerate(zip(row[2:],other[2:]))if x!=y])for row,other in zip(got,expected)if row!=other]
if errors:raise SystemExit(str(errors[:12])+f'\n{len(errors)} differing live states')
print('PASS128 source constructor inputs/2944 complete493-word live Executive/arithmetic states match unchanged assembly through AFC3433, bothSUPER forms')
print('Snapshot SHA256:',hashlib.sha256(run.stdout.encode()).hexdigest())
if not a.sanitize:
 run=subprocess.run(['build/p66-guidance-live-reference','deps/fixture/MAIN.agc.bin','--mixed'],capture_output=True,text=True,cwd=root,check=True);(root/'build/p66-guidance-live-mixed-reference.snapshots').write_text(run.stdout);(root/'build/p66-guidance-live-mixed-reference.trace').write_text(run.stderr)
run=subprocess.run(['build/'+name,'--mixed'],capture_output=True,text=True,cwd=root,check=True);mixed_name='p66-guidance-live-mixed-sanitize'if a.sanitize else 'p66-guidance-live-mixed-native';(root/'build'/f'{mixed_name}.snapshots').write_text(run.stdout)
assert run.stdout==(root/'build/p66-guidance-live-mixed-reference.snapshots').read_text(), 'Two real P66 jobs differ from original preemption state'
assert len(run.stdout.splitlines())==400
print('PASS16 two-real-P66-job preemptions/400 complete493-word states through higher-priority AFC; SHA256',hashlib.sha256(run.stdout.encode()).hexdigest())
