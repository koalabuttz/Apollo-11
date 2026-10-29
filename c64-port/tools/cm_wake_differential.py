#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Compare complete original states while retaining deterministic XZ streams."""
import pathlib,subprocess,hashlib,lzma,json,sys
R=pathlib.Path(__file__).resolve().parents[1];E=R/'deps/virtualagc/yaAGC'
def rows(p):return [' '.join(x.split('#',1)[0].split())for x in p.read_text().splitlines()if x.split('#',1)[0].strip()]
a=rows(R.parent/'Comanche055/EXECUTIVE.agc');b=rows(R/'deps/cm-services-fixture/EXECUTIVE.agc');assert a==b
for file,first,last in [('INTERPRETER.agc','NEG ','EXIT ')]:
 a=rows(R.parent/'Comanche055'/file);b=rows(R/'deps/cm-services-fixture'/file)
 a=a[next(i for i,x in enumerate(a)if x.startswith(first)):next(i for i,x in enumerate(a)if x.startswith(last))]
 b=b[next(i for i,x in enumerate(b)if x.startswith(first)):next(i for i,x in enumerate(b)if x.startswith(last))]
 assert a==b
base=['cc','-O2','-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-ffunction-sections','-Wl,--gc-sections','-Iinclude','-Itests']
cases=['src/agc_word.c','tests/cm_trig_cases.c','tests/cm_clockjob_cases.c','tests/cm_wake_cases.c']
subprocess.run(base+cases+['src/cm_basic_wake.c','src/cm_native_image.c','src/cm_dispatch.c','src/cm_executive.c','tools/cm_wake_snapshot.c','-o','build/cm-wake-native'],cwd=R,check=True)
subprocess.run([x for x in base if x!='-Wconversion']+['-I'+str(E)]+cases+['tools/cm_wake_reference.c','tests/scenarios.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-wake-reference'],cwd=R,check=True)
native=subprocess.Popen(['build/cm-wake-native'],cwd=R,stdout=subprocess.PIPE)
original=subprocess.Popen(['build/cm-wake-reference','deps/cm-services-fixture/MAIN.agc.bin'],cwd=R,stdout=subprocess.PIPE)
nh=hashlib.sha256();oh=hashlib.sha256();count=0;bad=0
with lzma.open(R/'build/cm-wake-native.snapshots.xz','wb',preset=6,check=lzma.CHECK_SHA256)as nf,lzma.open(R/'build/cm-wake-reference.snapshots.xz','wb',preset=6,check=lzma.CHECK_SHA256)as of:
 while True:
  x=native.stdout.readline();y=original.stdout.readline()
  if not x and not y:break
  if not x or not y:raise RuntimeError('stream length mismatch')
  nh.update(x);oh.update(y);nf.write(x);of.write(y);count+=1
  if x!=y:
   if bad<12:
    av=list(map(int,x.split()));bv=list(map(int,y.split()));print('DIFF',av[0],[(oct(i-2),oct(v),oct(w))for i,(v,w)in enumerate(zip(av,bv))if v!=w],flush=True)
   bad+=1
assert native.wait()==original.wait()==0
assert count==8192
(R/'build/cm-wake-stream-sha.json').write_text(json.dumps({'states':count,'native':nh.hexdigest(),'reference':oh.hexdigest(),'differing_states':bad},indent=2)+'\n')
if bad:sys.exit(str(bad)+' differing full source states')
print('PASS:8192 complete2050-word original CM JOBWAKE basic return states')
print('Uncompressed snapshot SHA256:',oh.hexdigest())
