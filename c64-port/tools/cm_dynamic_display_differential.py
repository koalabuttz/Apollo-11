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
core=['src/agc_multiword.c', 'src/agc_word.c', 'src/cm_basic_finish.c', 'src/cm_basic_priority.c', 'src/cm_basic_wake.c', 'src/cm_clockjob.c', 'src/cm_dispatch.c', 'src/cm_display_control.c', 'src/cm_dynamic_display.c', 'src/cm_dyndisp.c', 'src/cm_dyndisp_route.c', 'src/cm_executive.c', 'src/cm_formatter.c', 'src/cm_makeplay.c', 'src/cm_native_image.c', 'src/cm_nv85.c', 'src/cm_nv85_route.c', 'src/cm_pinball.c', 'src/cm_powered_axis.c', 'src/cm_powered_interface.c', 'src/cm_powered_trig.c', 'src/cm_rotation.c', 'src/cm_scale.c', 'src/dsky_decimal.c', 'src/dsky_pinball.c', 'src/dsky_scale.c', 'src/dsky_two_digit.c', 'src/gimbal_stage.c', 'src/guidance_product.c', 'src/guidance_shift.c', 'src/guidance_trig.c', 'src/matrix_native.c', 'tests/cm_clockjob_cases.c', 'tests/cm_dynamic_display_cases.c', 'tests/cm_dyndisp_apply.c', 'tests/cm_dyndisp_cases.c', 'tests/cm_nv85_apply.c', 'tests/cm_nv85_cases.c', 'tests/cm_trig_cases.c', 'tests/matrix_cases.c']
flags=['cc','-O2','-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','-ffunction-sections','-Wl,--gc-sections']
subprocess.run(flags+core+['tools/cm_dynamic_display_snapshot.c','-o','build/cm-dynamic-display-native'],cwd=R,check=True)
subprocess.run([x for x in flags if x!='-Wconversion']+['-I'+str(E)]+core+['tools/cm_dynamic_display_reference.c','tests/scenarios.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-dynamic-display-reference'],cwd=R,check=True)
a=subprocess.Popen(['build/cm-dynamic-display-native'],cwd=R,stdout=subprocess.PIPE);b=subprocess.Popen(['build/cm-dynamic-display-reference','deps/cm-powered-fixture/MAIN.agc.bin'],cwd=R,stdout=subprocess.PIPE);ah=hashlib.sha256();bh=hashlib.sha256();n=bad=0
with lzma.open(R/'build/cm-dynamic-display-native.snapshots.xz','wb',preset=3)as an,lzma.open(R/'build/cm-dynamic-display-reference.snapshots.xz','wb',preset=3)as bn:
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
(R/'build/cm-dynamic-display-stream-sha.json').write_text(json.dumps(dict(states=n,differing=bad,native=ah.hexdigest(),reference=bh.hexdigest()),indent=2)+'\n')
if bad:sys.exit(1)
