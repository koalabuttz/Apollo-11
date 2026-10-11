#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Original basic bodies through actual source END and native adopted idle."""
import argparse,hashlib,os,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--sanitize',action='store_true');a=p.parse_args()
sources=['src/p66_application.c','src/p66_launch_phased.c','src/p66_direct_launch.c','src/p66_native_guidance.c','src/p66_guidance_invoke.c','src/p66_descent_runtime.c','src/p66_descent_prefix_control.c','src/p66_descent_invoke.c','src/guidance_control_observer.c','src/p66_basic.c','src/dsky_display.c']
for file in ('display_job','p66_launch','p66_mode_display','dsky_charin_nv','dsky_charin_number'):
 for line in (ROOT/f'tools/{file}.mk').read_text().splitlines():
  if '_CORE = ' in line:sources.extend(name for name in line.split(' = ',1)[1].split() if name.startswith('src/'))
sources=list(dict.fromkeys(sources))
factories=['tests/p66_bias_cases.c','tests/p66_mode_display_cases.c','tests/dsky_nv_key_cases.c','tests/dsky_charin_nv_cases.c','tests/dsky_charin_number_cases.c','tests/p66_application_job_cases.c']
flags=['gcc','-std=c99','-O1' if a.sanitize else '-O2','-Wall','-Wextra','-Werror','-Iinclude','-Itests','-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-DP66_NATIVE_DISPATCH_ONLY','-DP66_GUIDANCE_DISPATCH_ONLY','-DP66_BASIC_DISPATCH_ONLY','-DP66_DIRECT_LAUNCH_DISPATCH_ONLY']
stem='p66-application-job-sanitize' if a.sanitize else 'p66-application-job-native'
if a.sanitize:flags+=['-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']
subprocess.run(flags+sources+factories+['tools/p66_application_job_snapshot.c','-o','build/'+stem],cwd=ROOT,check=True)
def capture(command,name):
 with (ROOT/f'build/{name}.snapshots').open('w') as out,(ROOT/f'build/{name}.trace').open('w') as err:subprocess.run(command,cwd=ROOT,stdout=out,stderr=err,check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
if not a.sanitize:
 engine=ROOT/'deps/virtualagc/yaAGC'
 subprocess.run(['gcc','-std=c99','-O2','-Iinclude','-Itests','-I'+str(engine),'tools/p66_application_job_reference.c','tests/scenarios.c']+factories+[str(engine/f) for f in ('agc_engine.o','agc_engine_init.o','rfopen.o')]+['-o','build/p66-application-job-reference'],cwd=ROOT,check=True)
 capture(['build/p66-application-job-reference','deps/fixture/MAIN.agc.bin'],'p66-application-job-reference')
capture(['build/'+stem],stem)
expected=ROOT/'build/p66-application-job-reference.snapshots';actual=ROOT/f'build/{stem}.snapshots'
with expected.open() as e,actual.open() as g:
 for n,(left,right) in enumerate(zip(e,g)):
  if left!=right:
   x=list(map(int,right.split()));y=list(map(int,left.split()));diff=[(oct(i-1),oct(v),oct(w))for i,(v,w)in enumerate(zip(x,y))if v!=w]
   raise SystemExit(f'case{n} native/original {diff[:30]}')
 assert n+1==13312 and not e.readline() and not g.readline()
print('PASS:13312 real deferred mode/VERB/NOUN/digit jobs, full2052 raw words through canonical ENDOFJOB and source idle; peripheral channel11 synchronized')
print('Snapshot SHA256:',hashlib.sha256(expected.read_bytes()).hexdigest())
