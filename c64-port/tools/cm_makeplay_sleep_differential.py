#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Compare complete original states while retaining deterministic XZ streams."""
import pathlib,subprocess,hashlib,lzma,json,sys
R=pathlib.Path(__file__).resolve().parents[1];E=R/'deps/virtualagc/yaAGC'
def rows(p):return [' '.join(x.split('#',1)[0].split())for x in p.read_text().splitlines()if x.split('#',1)[0].strip()]
a=rows(R.parent/'Comanche055/DISPLAY_INTERFACE_ROUTINES.agc');b=rows(R/'deps/cm-services-fixture/DISPLAY_INTERFACE_ROUTINES.agc')
for file,first,last in [('DISPLAY_INTERFACE_ROUTINES.agc','GOSLEEPS ','JOBXCHS '),('DISPLAY_INTERFACE_ROUTINES.agc','XCHTOEND ','XCHNYLOC '),('SERVICE_ROUTINES.agc','UPENT2 ','DOWNENT2 ')]:
 a=rows(R.parent/'Comanche055'/file);b=rows(R/'deps/cm-services-fixture'/file)
 av=a[next(i for i,x in enumerate(a)if x.startswith(first)):next(i for i,x in enumerate(a)if x.startswith(last))]
 bv=b[next(i for i,x in enumerate(b)if x.startswith(first)):next(i for i,x in enumerate(b)if x.startswith(last))]
 assert av==bv,(first,last)
base=['cc','-O2','-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-ffunction-sections','-Wl,--gc-sections','-Iinclude','-Itests']
cases=['src/agc_word.c','tests/cm_trig_cases.c','tests/cm_clockjob_cases.c','tests/cm_display_control_cases.c','tests/cm_makeplay_sleep_cases.c']
subprocess.run(base+cases+['src/cm_makeplay_sleep.c','tools/cm_makeplay_sleep_snapshot.c','-o','build/cm-makeplay-sleep-native'],cwd=R,check=True)
subprocess.run([x for x in base if x!='-Wconversion']+['-I'+str(E)]+cases+['tools/cm_makeplay_sleep_reference.c','tests/scenarios.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-makeplay-sleep-reference'],cwd=R,check=True)
native=subprocess.Popen(['build/cm-makeplay-sleep-native'],cwd=R,stdout=subprocess.PIPE)
original=subprocess.Popen(['build/cm-makeplay-sleep-reference','deps/cm-services-fixture/MAIN.agc.bin'],cwd=R,stdout=subprocess.PIPE)
nh=hashlib.sha256();oh=hashlib.sha256();count=0;bad=0
with lzma.open(R/'build/cm-makeplay-sleep-native.snapshots.xz','wb',preset=3,check=lzma.CHECK_SHA256)as nf,lzma.open(R/'build/cm-makeplay-sleep-reference.snapshots.xz','wb',preset=3,check=lzma.CHECK_SHA256)as of:
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
assert count==6144
(R/'build/cm-makeplay-sleep-stream-sha.json').write_text(json.dumps({'states':count,'native':nh.hexdigest(),'reference':oh.hexdigest(),'differing_states':bad},indent=2)+'\n')
if bad:sys.exit(str(bad)+' differing full source states')
print('PASS:6144 complete2051-word original CM MAKEPLAY priority, normal/clock selection and wake-return states')
print('Uncompressed snapshot SHA256:',oh.hexdigest())
