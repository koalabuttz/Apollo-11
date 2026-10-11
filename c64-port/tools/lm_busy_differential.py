#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,hashlib,sys
root=pathlib.Path(__file__).resolve().parents[1];engine=root/'deps/virtualagc/yaAGC'
def rows(p):return [' '.join(x.split('#',1)[0].split())for x in p.read_text().splitlines()if x.split('#',1)[0].strip()]
for f in ['INTERPRETER.agc','INTER-BANK_COMMUNICATION.agc','DISPLAY_INTERFACE_ROUTINES.agc','EXECUTIVE.agc','PINBALL_GAME_BUTTONS_AND_LIGHTS.agc']:
 a=rows(root.parent/'Luminary099'/f);b=rows(root/'deps/fixture'/('PINBALL_GAME__BUTTONS_AND_LIGHTS.agc' if f=='PINBALL_GAME_BUTTONS_AND_LIGHTS.agc' else f))
 if f=='INTER-BANK_COMMUNICATION.agc':
  a=a[next(i for i,x in enumerate(a)if x.startswith('POSTJUMP ')):next(i for i,x in enumerate(a)if x.startswith('MAKECADR '))];b=b[next(i for i,x in enumerate(b)if x.startswith('POSTJUMP ')):next(i for i,x in enumerate(b)if x.startswith('MAKECADR '))]
 if f=='DISPLAY_INTERFACE_ROUTINES.agc':
  # Fixture relocates this unusedalias declaration toavoidduplicate modernsymbol; no instructionschanged.
  assert [x for x in a if x!='PRIOSAVE EQUALS R1SAVE']==b,f
 elif f=='PINBALL_GAME_BUTTONS_AND_LIGHTS.agc':assert [x.replace('DSPOCTWO','DSPOCTWD') for x in a]==b,f
 else:assert a==b,f
base=['gcc','-std=c99','-O2','-Iinclude','-Itests']
subprocess.run(base+['src/agc_word.c','src/lm_pinball_busy.c','tests/throttle_bridge_cases.c','tests/lm_busy_cases.c','tools/lm_busy_snapshot.c','-o','build/lm-busy-native'],cwd=root,check=True)
subprocess.run(base+['-I'+str(engine),'src/agc_word.c','src/lm_pinball_busy.c','tests/throttle_bridge_cases.c','tests/lm_busy_cases.c','tools/lm_busy_reference.c','tests/scenarios.c',str(engine/'agc_engine.o'),str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/lm-busy-reference'],cwd=root,check=True)
output=[]
for name,cmd in [('native',['build/lm-busy-native']),('reference',['build/lm-busy-reference','deps/fixture/MAIN.agc.bin'])]:
 p=subprocess.run(cmd,cwd=root,capture_output=True,text=True,check=True);(root/f'build/lm-busy-{name}.snapshots').write_text(p.stdout);output.append(p.stdout.splitlines())
assert len(output[0])==len(output[1])==8192
bad=[(a,b)for a,b in zip(*output)if a!=b]
for a,b in bad[:12]:
 av=list(map(int,a.split()));bv=list(map(int,b.split()));print('DIFF',av[0],[(oct(i-2),oct(x),oct(y))for i,(x,y)in enumerate(zip(av,bv))if x!=y])
if bad:sys.exit(f'{len(bad)} differing states')
print('PASS:8192 complete2048-word original source LM NVSUBUSY/POSTJUMP/sleep-or-alarm, plus SUPER/IRQ')
print('Snapshot SHA256:',hashlib.sha256((root/'build/lm-busy-reference.snapshots').read_bytes()).hexdigest())
