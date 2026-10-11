#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,hashlib,lzma,json,sys
R=pathlib.Path(__file__).resolve().parents[1];E=R/'deps/virtualagc/yaAGC';base=['cc','-O2','-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-ffunction-sections','-Wl,--gc-sections','-Iinclude','-Itests'];cases=['src/agc_word.c','tests/matrix_cases.c']
subprocess.run(base+cases+['src/agc_multiword.c','src/matrix_native.c','src/guidance_product.c','tools/matrix_snapshot.c','-o','build/matrix-native'],cwd=R,check=True)
subprocess.run([x for x in base if x!='-Wconversion']+['-I'+str(E)]+cases+['tools/matrix_reference.c','tests/scenarios.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/matrix-reference'],cwd=R,check=True)
def normalized(p):return [' '.join(line.split('#',1)[0].split())for line in p.read_text().splitlines()if line.split('#',1)[0].strip()]
for mission,fixture in [('Comanche055','cm-powered-fixture'),('Luminary099','fixture')]:
 a=normalized(R.parent/mission/'INTERPRETER.agc');b=normalized(R/'deps'/fixture/'INTERPRETER.agc')
 for first,last in [('OVERFLOW ','BVSU '),('DMPSUB ','ROUNDSUB '),('DOTSUB ','POWRSERS '),('MXV ','VXSC '),('MPACVBUF ','ALSIGNAG ')]:
  av=a[next(i for i,x in enumerate(a)if x.startswith(first)):next(i for i,x in enumerate(a)if x.startswith(last))]
  bv=b[next(i for i,x in enumerate(b)if x.startswith(first)):next(i for i,x in enumerate(b)if x.startswith(last))]
  assert av==bv,(mission,first,last)
for mission,rope in [('CM','deps/cm-powered-fixture/MAIN.agc.bin'),('LM','deps/fixture/MAIN.agc.bin')]:
 native=subprocess.Popen(['build/matrix-native',mission],cwd=R,stdout=subprocess.PIPE);original=subprocess.Popen(['build/matrix-reference',rope,mission],cwd=R,stdout=subprocess.PIPE);nh=hashlib.sha256();oh=hashlib.sha256();count=bad=0
 with lzma.open(R/f'build/matrix-{mission}-native.snapshots.xz','wb',preset=3,check=lzma.CHECK_SHA256)as nf,lzma.open(R/f'build/matrix-{mission}-reference.snapshots.xz','wb',preset=3,check=lzma.CHECK_SHA256)as of:
  while True:
   x=native.stdout.readline();y=original.stdout.readline()
   if not x and not y:break
   if not x or not y:raise RuntimeError('stream length mismatch')
   nh.update(x);oh.update(y);nf.write(x);of.write(y);count+=1
   if x!=y:
    if bad<10:
     av=list(map(int,x.split()));bv=list(map(int,y.split()));print('DIFF',mission,av[0],[(oct(i-2),oct(v),oct(w))for i,(v,w)in enumerate(zip(av,bv))if v!=w],flush=True)
    bad+=1
 assert native.wait()==original.wait()==0
 (R/f'build/matrix-{mission}-stream-sha.json').write_text(json.dumps({'states':count,'native':nh.hexdigest(),'reference':oh.hexdigest(),'differing_states':bad},indent=2)+'\n')
 if bad:sys.exit(str(bad)+' differing source states')
 print('PASS',mission,count,'complete2050-word MXV/VXM phases; SHA256',oh.hexdigest(),flush=True)
