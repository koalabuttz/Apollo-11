#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Compare original idle ADVAN basic-entry transfers across all erasable words."""
import argparse,hashlib,os,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--sanitize',action='store_true');a=p.parse_args()
sources=['src/agc_word.c','src/lm_idle_start.c']
factories=['tests/lm_idle_start_cases.c']
flags=['gcc','-std=c99','-O1' if a.sanitize else '-O2','-Wall','-Wextra','-Werror','-Iinclude','-Itests','-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-DP66_NATIVE_DISPATCH_ONLY','-DP66_GUIDANCE_DISPATCH_ONLY','-DP66_BASIC_DISPATCH_ONLY','-DP66_DIRECT_LAUNCH_DISPATCH_ONLY']
stem='lm-idle-start-sanitize' if a.sanitize else 'lm-idle-start-native'
if a.sanitize:flags+=['-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']
subprocess.run(flags+sources+factories+['tools/lm_idle_start_snapshot.c','-o','build/'+stem],cwd=ROOT,check=True)
def capture(command,name):
 with (ROOT/f'build/{name}.snapshots').open('w') as out,(ROOT/f'build/{name}.trace').open('w') as err:subprocess.run(command,cwd=ROOT,stdout=out,stderr=err,check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
if not a.sanitize:
 engine=ROOT/'deps/virtualagc/yaAGC'
 subprocess.run(['gcc','-std=c99','-O2','-Iinclude','-Itests','-I'+str(engine),'tools/lm_idle_start_reference.c','tests/scenarios.c']+factories+[str(engine/f) for f in ('agc_engine.o','agc_engine_init.o','rfopen.o')]+['-o','build/lm-idle-start-reference'],cwd=ROOT,check=True)
 capture(['build/lm-idle-start-reference','deps/fixture/MAIN.agc.bin'],'lm-idle-start-reference')
capture(['build/'+stem],stem)
expected=ROOT/'build/lm-idle-start-reference.snapshots';actual=ROOT/f'build/{stem}.snapshots'
with expected.open() as e,actual.open() as g:
 for n,(left,right) in enumerate(zip(e,g)):
  if left!=right:
   x=list(map(int,right.split()));y=list(map(int,left.split()));diff=[(oct(i-1),oct(v),oct(w))for i,(v,w)in enumerate(zip(x,y))if v!=w]
   raise SystemExit(f'case{n} native/original {diff[:30]}')
 assert n+1==8192 and not e.readline() and not g.readline()
print('PASS:8192 original idle ADVAN→NUDIRECT→SUPDXCHZ source transfers, full2052 raw words including actual queued RODCOMP admission destinations')
print('Snapshot SHA256:',hashlib.sha256(expected.read_bytes()).hexdigest())
