#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Pending jobs hand off at every original EDOP0 completed-pair boundary."""
import argparse,pathlib,subprocess,hashlib
root=pathlib.Path(__file__).resolve().parents[1];p=argparse.ArgumentParser();p.add_argument('--sanitize',action='store_true');a=p.parse_args()
inputs=[f'tests/{name}_cases.c'for name in ('prepare','scale','push','elapsed','store','divide','gravity','velocity','radial','unit','trig','projection','p66_guidance_live')]
defines=['-D'+name+'_INPUT_ONLY'for name in ('PREPARE','SCALE','PUSH','ELAPSED','STORE','DIVIDE','GRAVITY','VELOCITY','RADIAL','UNIT','TRIG','PROJECTION')]
numerical=['src/guidance.c']+[f'src/guidance_{name}.c'for name in ('velocity_phased','vector','control_observer','prepare','scale','push','elapsed','store','divide','gravity','product','shift','velocity','dp_divide','norm','unit','unit_phased','dot','sum','radial','radial_phased','acceleration','acceleration_phased','acceleration_store','trig','rotation','projection','projection_phased')]
base=['gcc','-std=c99','-O1'if a.sanitize else '-O2','-Wall','-Wextra','-Werror','-ffunction-sections','-Wl,--gc-sections','-Iinclude','-Itests']
if a.sanitize:base+=['-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']

name='p66-guidance-pending-sanitize'if a.sanitize else 'p66-guidance-pending-native'
subprocess.run(base+defines+['platform/host/p66_guidance_pending_snapshot.c','src/agc_word.c','src/agc_multiword.c','src/executive.c','src/native_dispatch.c','src/irq_delivery.c','src/p66_native.c','src/p66_native_guidance.c','src/p66_guidance_invoke.c']+numerical+inputs+['-o','build/'+name],cwd=root,check=True)
if not a.sanitize:
 for source in ('EXECUTIVE.agc','INTERPRETER.agc','LUNAR_LANDING_GUIDANCE_EQUATIONS.agc'):
  assert (root.parent/'Luminary099'/source).read_bytes()==(root/'deps/fixture'/source).read_bytes(),source
 subprocess.run(['python3','tools/prepare_hardware_dv.py'],cwd=root,check=True)
 engine=root/'deps/virtualagc/yaAGC'
 subprocess.run(base+defines+['-DP66_NATIVE_DISPATCH_ONLY','-I'+str(engine),'tools/p66_guidance_pending_reference.c','tests/scenarios.c','src/agc_word.c','src/agc_multiword.c','src/executive.c','src/p66_native.c']+inputs+['build/agc_engine_hardware_dv.o',str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/p66-guidance-pending-reference'],cwd=root,check=True)
 run=subprocess.run(['build/p66-guidance-pending-reference','deps/fixture/MAIN.agc.bin'],capture_output=True,text=True,cwd=root,check=True);(root/'build/p66-guidance-pending-reference.snapshots').write_text(run.stdout)
run=subprocess.run(['build/'+name],capture_output=True,text=True,cwd=root,check=True);(root/'build'/f'{name}.snapshots').write_text(run.stdout)
expected=(root/'build/p66-guidance-pending-reference.snapshots').read_text()
if run.stdout!=expected:
 got=[list(map(int,l.split()))for l in run.stdout.splitlines()];ref=[list(map(int,l.split()))for l in expected.splitlines()]
 diff=[(g[:3],[(i,x,y)for i,(x,y)in enumerate(zip(g[3:],r[3:]))if x!=y])for g,r in zip(got,ref)if g!=r]
 raise SystemExit(str(diff[:8])+f' differing rows {len(diff)}')
assert len(expected.splitlines())==704
print('PASS352 pending NEWJOB injections at22 source-safe boundaries:704 complete493-word source/native checkpoints; SHA256',hashlib.sha256(run.stdout.encode()).hexdigest())
