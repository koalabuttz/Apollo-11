#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Retain compressed full original admission states; compare native streaming."""
import argparse,hashlib,itertools,lzma,os,subprocess
from pathlib import Path
root=Path(__file__).resolve().parents[1];p=argparse.ArgumentParser();p.add_argument('--sanitize',action='store_true');p.add_argument('--retained',action='store_true');a=p.parse_args();base=['gcc','-std=c99','-O1'if a.sanitize else'-O2','-Wall','-Wextra','-Werror','-Iinclude','-Itests','-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-DP66_NATIVE_DISPATCH_ONLY'];source=['src/agc_word.c','src/executive.c','src/p66_native.c','tests/lm_admission_image_cases.c','tests/lm_admission_raw_cases.c','tests/throttle_bridge_cases.c','tests/lm_wake_cases.c','tests/lm_wake_image_cases.c','tests/lm_spectest_handoff_cases.c'];name='lm-spectest-handoff-sanitize'if a.sanitize else'lm-spectest-handoff-native'
if a.sanitize:base+=['-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']
subprocess.run(base+source+['src/lm_spectest_handoff.c','tools/lm_spectest_handoff_snapshot.c','-o','build/'+name],cwd=root,check=True);archive=root/('evidence/lm-spectest-handoff-reference.snapshots.xz'if a.retained else'build/lm-spectest-handoff-reference.snapshots.xz')
if not a.sanitize and not a.retained:
 engine=root/'deps/virtualagc/yaAGC';subprocess.run(base+['-I'+str(engine),'tools/lm_spectest_handoff_reference.c','tests/scenarios.c']+source+[str(engine/f)for f in('agc_engine.o','agc_engine_init.o','rfopen.o')]+['-o','build/lm-spectest-handoff-reference'],cwd=root,check=True)
 with archive.open('wb')as raw,lzma.open(raw,'wb',preset=1)as out:
  process=subprocess.Popen(['build/lm-spectest-handoff-reference','deps/fixture/MAIN.agc.bin'],cwd=root,stdout=subprocess.PIPE)
  for line in process.stdout:out.write(line)
  assert process.wait()==0
h=hashlib.sha256();errors=[]
with lzma.open(archive,'rb')as expected:
 process=subprocess.Popen(['build/'+name],cwd=root,stdout=subprocess.PIPE,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
 try:
  for n,(x,y)in enumerate(itertools.zip_longest(expected,process.stdout),1):
   if x!=y:
    xx=list(map(int,x.split()))if x else[];yy=list(map(int,y.split()))if y else[];errors.append((n-1,[(oct(i-2),oct(u),oct(v))for i,(u,v)in enumerate(zip(xx,yy))if u!=v]));
   h.update(x)
  assert process.wait()==0
 finally:
  if process.poll()is None:process.kill();process.wait()
  process.stdout.close()
if errors:raise SystemExit(str(errors[:12])+f'\n{len(errors)} differing complete states')
assert n==16384
print('PASS:16384 original admission/wake/SPECTEST complete2050-word actual CCSHOLE5705 handoffs')
print('Snapshot SHA256:',h.hexdigest())
