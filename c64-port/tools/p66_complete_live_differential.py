#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Execute real basic/interpretive descent and compare historical checkpoints."""
import argparse,pathlib,subprocess,hashlib,os
root=pathlib.Path(__file__).resolve().parents[1];p=argparse.ArgumentParser();p.add_argument('--sanitize',action='store_true');p.add_argument('--raw-control',action='store_true');a=p.parse_args()
inputs=[f'tests/{name}_cases.c'for name in('prepare','scale','push','elapsed','store','divide','gravity','velocity','radial','unit','trig','projection','p66_guidance_live','p66_descent_live','p66_complete_live')]
defines=['-D'+name+'_INPUT_ONLY'for name in('PREPARE','SCALE','PUSH','ELAPSED','STORE','DIVIDE','GRAVITY','VELOCITY','RADIAL','UNIT','TRIG','PROJECTION')]
numerical=['src/guidance.c']+[f'src/guidance_{name}.c'for name in('velocity_phased','vector','control_observer','prepare','scale','push','elapsed','store','divide','gravity','product','shift','velocity','dp_divide','norm','unit','unit_phased','dot','sum','radial','radial_phased','acceleration','acceleration_phased','acceleration_store','trig','rotation','projection','projection_phased','horizontal','horizontal_phased','feedback_pairs','length_phased','throttle_service')]
base=['gcc','-std=c99','-O1'if a.sanitize else'-O2','-Wall','-Wextra','-Werror','-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-Iinclude','-Itests']
if a.raw_control:base+=['-DPCL_RAW_CONTROL']
if a.sanitize:base+=['-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']
name='p66-complete-live-sanitize'if a.sanitize else'p66-complete-live-native'
if a.raw_control:name+='-raw'
native=['src/agc_word.c','src/agc_multiword.c','src/executive.c','src/native_dispatch.c','src/irq_delivery.c','src/p66_native.c','src/p66_native_guidance.c','src/p66_guidance_invoke.c','src/p66_basic.c','src/p66_descent_runtime.c','src/p66_descent_prefix_control.c','src/p66_native_descent.c','src/p66_descent_invoke.c','src/p66_descent_control.c','src/agc_hardware_divide.c','src/p66_initialization.c','src/p66_runtime_bridge.c','src/p66_runtime_fetch_bridge.c','src/p66_runtime_regodspr.c']
native+=['src/p66_startup.c','src/phase.c','src/waitlist.c']
extra=(root/'tools/display_job.mk').read_text().split('DJOB_CORE = ',1)[1].split('\n',1)[0].split()
native=list(dict.fromkeys(native+[f for f in extra if f.startswith('src/')]))
numerical=[f for f in numerical if f not in native]
subprocess.run(base+defines+['-DP66_BASIC_DISPATCH_ONLY','platform/host/p66_complete_live_snapshot.c']+native+numerical+inputs+['-o','build/'+name],cwd=root,check=True)
if not a.sanitize:
 for source in('EXECUTIVE.agc','INTERPRETER.agc','LUNAR_LANDING_GUIDANCE_EQUATIONS.agc'):assert(root.parent/'Luminary099'/source).read_bytes()==(root/'deps/fixture'/source).read_bytes(),source
 subprocess.run(['python3','tools/prepare_hardware_dv.py'],cwd=root,check=True)
 engine=root/'deps/virtualagc/yaAGC'
 subprocess.run(base+defines+['-DP66_NATIVE_DISPATCH_ONLY','-I'+str(engine),'tools/p66_complete_live_reference.c','tests/scenarios.c','src/agc_word.c','src/agc_multiword.c','src/executive.c','src/p66_native.c']+inputs+['build/agc_engine_hardware_dv.o',str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/p66-complete-live-reference'+('-raw'if a.raw_control else'')],cwd=root,check=True)
 result=subprocess.run(['build/p66-complete-live-reference'+('-raw'if a.raw_control else''),'deps/fixture/MAIN.agc.bin'],capture_output=True,text=True,cwd=root,check=True)
 (root/('build/p66-complete-live-reference'+('-raw'if a.raw_control else'')+'.snapshots')).write_text(result.stdout);(root/'build/p66-complete-live-reference.trace').write_text(result.stderr)
run=subprocess.run(['build/'+name],capture_output=True,text=True,cwd=root,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
(root/'build'/f'{name}.snapshots').write_text(run.stdout);(root/'build'/f'{name}.trace').write_text(run.stderr)
if run.returncode:raise SystemExit(run.stderr)
got=[list(map(int,l.split()))for l in run.stdout.splitlines()];expected=[list(map(int,l.split()))for l in(root/('build/p66-complete-live-reference'+('-raw'if a.raw_control else'')+'.snapshots')).read_text().splitlines()]
if len(got)!=len(expected):raise SystemExit(f'Checkpoint count native{len(got)} original{len(expected)}')
errors=[(row[:3],[(i,x,y)for i,(x,y)in enumerate(zip(row[3:],other[3:]))if x!=y])for row,other in zip(got,expected)if row!=other]
if errors:raise SystemExit(str(errors[:12])+f'\n{len(errors)} differing live states')
for modes in(('--resolved',),('--services',),('--resolved','--services')):
 alternative=subprocess.run(['build/'+name,*modes],capture_output=True,text=True,cwd=root,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
 if alternative.returncode:raise SystemExit(alternative.stderr or f'Failure mode{modes}')
 assert alternative.stdout==run.stdout,modes
print('Linked registry, compiledresolver and bound locked service seams all match the same original checkpoints')
print('PASS16 actual complete enabled-display P66 paths/','%d'%len(got),'source-qualified2052word Executive/math/control states through realENDOFJOB andnativeidle')
print('Snapshot SHA256:',hashlib.sha256(run.stdout.encode()).hexdigest())
