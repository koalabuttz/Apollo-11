#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma,hashlib,sys,json
R=pathlib.Path(__file__).resolve().parents[1];E=R/'deps/virtualagc/yaAGC'
def normalized(p):return [' '.join(x.split('#',1)[0].split()) for x in p.read_text().splitlines() if x.split('#',1)[0].strip()]
a=normalized(R.parent/'Comanche055/P40-P47.agc');b=normalized(R/'deps/cm-powered-fixture/P40-P47.agc')
for first,last in [('P40CNV85 ','CALCN85 '),('DYNDISP ','CLOCKJOB '),('S41.1 ','TENBNK14 ')]:
 ac=a[next(i for i,x in enumerate(a)if x.startswith(first)):next(i for i,x in enumerate(a)if x.startswith(last))]
 bc=b[next(i for i,x in enumerate(b)if x.startswith(first)):next(i for i,x in enumerate(b)if x.startswith(last))]
 assert ac==bc,(first,last)
core=['src/agc_word.c','src/agc_multiword.c','src/guidance_product.c','src/guidance_shift.c','src/cm_dyndisp.c','src/cm_dyndisp_route.c','src/matrix_native.c','src/gimbal_stage.c','src/cm_powered_interface.c','src/cm_powered_axis.c','src/cm_powered_trig.c','src/cm_rotation.c','src/guidance_trig.c','tests/matrix_cases.c','tests/cm_dyndisp_cases.c','tests/cm_dyndisp_apply.c']
flags=['cc','-O2','-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','-ffunction-sections','-Wl,--gc-sections']
subprocess.run(flags+core+['tools/cm_dyndisp_snapshot.c','-o','build/cm-dyndisp-native'],cwd=R,check=True)
subprocess.run([x for x in flags if x!='-Wconversion']+['-I'+str(E)]+core+['tools/cm_dyndisp_reference.c','tests/scenarios.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-dyndisp-reference'],cwd=R,check=True)
a=subprocess.Popen(['build/cm-dyndisp-native'],cwd=R,stdout=subprocess.PIPE);b=subprocess.Popen(['build/cm-dyndisp-reference','deps/cm-powered-fixture/MAIN.agc.bin'],cwd=R,stdout=subprocess.PIPE);ah=hashlib.sha256();bh=hashlib.sha256();n=bad=0
with lzma.open(R/'build/cm-dyndisp-native.snapshots.xz','wb',preset=3)as an,lzma.open(R/'build/cm-dyndisp-reference.snapshots.xz','wb',preset=3)as bn:
 while True:
  x=a.stdout.readline();y=b.stdout.readline()
  if not x and not y:break
  if not x or not y:raise RuntimeError('length mismatch '+str(n))
  ah.update(x);bh.update(y);an.write(x);bn.write(y);n+=1
  if x!=y:
   if bad<25:
    av=list(map(int,x.split()));bv=list(map(int,y.split()));print('DIFF',av[0],[(oct(i-2),oct(v),oct(w))for i,(v,w)in enumerate(zip(av,bv))if v!=w],flush=True)
   bad+=1
assert a.wait()==b.wait()==0
print(n,bad,ah.hexdigest(),bh.hexdigest(),flush=True)
(R/'build/cm-dyndisp-stream-sha.json').write_text(json.dumps(dict(states=n,differing=bad,native=ah.hexdigest(),reference=bh.hexdigest()),indent=2)+'\n')
if bad:sys.exit(1)
