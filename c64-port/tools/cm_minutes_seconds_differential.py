#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma,hashlib,json,sys
R=pathlib.Path(__file__).resolve().parents[1];E=R/'deps/virtualagc/yaAGC'
def normalized(p):return [' '.join(x.split('#',1)[0].split()) for x in p.read_text().splitlines() if x.split('#',1)[0].strip()]
for original,fixture in [('DISPLAY_INTERFACE_ROUTINES.agc','DISPLAY_INTERFACE_ROUTINES.agc'),('PINBALL_GAME_BUTTONS_AND_LIGHTS.agc','PINBALL_GAME__BUTTONS_AND_LIGHTS.agc'),('PINBALL_NOUN_TABLES.agc','PINBALL_NOUN_TABLES.agc'),('EXECUTIVE.agc','EXECUTIVE.agc')]:
 a=normalized(R.parent/'Comanche055'/original);b=normalized(R/'deps/cm-powered-fixture'/fixture)
 if original=='DISPLAY_INTERFACE_ROUTINES.agc':
  assert a.count('CAF EBANKSAV')==1 and a.count('PRIOSAVE EQUALS R1SAVE')==1
  a=[x.replace('CAF EBANKSAV','CA EBANKSAV') for x in a if x!='PRIOSAVE EQUALS R1SAVE']
 if original=='PINBALL_GAME_BUTTONS_AND_LIGHTS.agc':
  assert sum('DSPOCTWO' in x for x in a)==4
  a=[x.replace('DSPOCTWO','DSPOCTWD') for x in a]
 assert a==b,original
assert hashlib.sha256((R/'deps/cm-powered-fixture/MAIN.agc.bin').read_bytes()).hexdigest()=='6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061'
core=['src/cm_minutes_seconds.c', 'src/cm_decimal_pair.c', 'src/cm_pinball.c', 'src/dsky_pinball.c', 'src/cm_interbank.c', 'src/fixed_data.c', 'src/guidance_product.c', 'src/agc_word.c', 'src/agc_multiword.c', 'tests/cm_minutes_seconds_cases.c']
flags=['cc','-O2','-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','-ffunction-sections','-Wl,--gc-sections']
subprocess.run(flags+core+['tools/cm_minutes_seconds_snapshot.c','-o','build/cm-minutes-seconds-native'],cwd=R,check=True)
subprocess.run([x for x in flags if x!='-Wconversion']+['-I'+str(E)]+core+['tools/cm_minutes_seconds_reference.c','tests/scenarios.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-minutes-seconds-reference'],cwd=R,check=True)
a=subprocess.Popen(['build/cm-minutes-seconds-native'],cwd=R,stdout=subprocess.PIPE);b=subprocess.Popen(['build/cm-minutes-seconds-reference','deps/cm-powered-fixture/MAIN.agc.bin'],cwd=R,stdout=subprocess.PIPE);ah=hashlib.sha256();bh=hashlib.sha256();n=bad=0
with lzma.open(R/'build/cm-minutes-seconds-native.snapshots.xz','wb',preset=3)as an,lzma.open(R/'build/cm-minutes-seconds-reference.snapshots.xz','wb',preset=3)as bn:
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
(R/'build/cm-minutes-seconds-stream-sha.json').write_text(json.dumps(dict(states=n,differing=bad,native=ah.hexdigest(),reference=bh.hexdigest()),indent=2)+'\n')
if bad:sys.exit(1)
