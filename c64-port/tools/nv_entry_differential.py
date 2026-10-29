#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,hashlib,sys
root=pathlib.Path(__file__).resolve().parents[1];engine=root/'deps/virtualagc/yaAGC'
def rows(p):return [' '.join(x.split('#',1)[0].split())for x in p.read_text().splitlines()if x.split('#',1)[0].strip()]
for f in ['INTERPRETER.agc','INTER-BANK_COMMUNICATION.agc','PINBALL_GAME_BUTTONS_AND_LIGHTS.agc']:
 a=rows(root.parent/'Luminary099'/f);b=rows(root/'deps/fixture'/('PINBALL_GAME__BUTTONS_AND_LIGHTS.agc' if f=='PINBALL_GAME_BUTTONS_AND_LIGHTS.agc' else f))
 if f=='INTER-BANK_COMMUNICATION.agc':
  a=a[next(i for i,x in enumerate(a)if x.startswith('BANKCALL ')):next(i for i,x in enumerate(a)if x.startswith('POSTJUMP '))];b=b[next(i for i,x in enumerate(b)if x.startswith('BANKCALL ')):next(i for i,x in enumerate(b)if x.startswith('POSTJUMP '))]
 if f=='PINBALL_GAME_BUTTONS_AND_LIGHTS.agc':a=[x.replace('DSPOCTWO','DSPOCTWD')for x in a]
 assert a==b,f
base=['gcc','-std=c99','-O2','-Iinclude','-Itests']
subprocess.run(base+['src/agc_word.c','src/p66_nv_entry.c','tests/throttle_bridge_cases.c','tests/nv_entry_cases.c','tools/nv_entry_snapshot.c','-o','build/nv-entry-native'],cwd=root,check=True)
subprocess.run(base+['-I'+str(engine),'tests/throttle_bridge_cases.c','tests/nv_entry_cases.c','tools/nv_entry_reference.c','tests/scenarios.c',str(engine/'agc_engine.o'),str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/nv-entry-reference'],cwd=root,check=True)
output=[]
for name,cmd in [('native',['build/nv-entry-native']),('reference',['build/nv-entry-reference','deps/fixture/MAIN.agc.bin'])]:
 p=subprocess.run(cmd,cwd=root,capture_output=True,text=True,check=True);(root/f'build/nv-entry-{name}.snapshots').write_text(p.stdout);output.append(p.stdout.splitlines())
assert len(output[0])==len(output[1])==8192
bad=[(a,b)for a,b in zip(*output)if a!=b]
for a,b in bad[:12]:
 av=list(map(int,a.split()));bv=list(map(int,b.split()));print('DIFF',av[0],[(oct(i-2),oct(x),oct(y))for i,(x,y)in enumerate(zip(av,bv))if x!=y])
if bad:sys.exit(f'{len(bad)} differing states')
print('PASS:8192 complete2048-word original V06N60 NVMONOPT busy/UPDATVB entry branches, plus SUPER/IRQ')
print('Snapshot SHA256:',hashlib.sha256((root/'build/nv-entry-reference.snapshots').read_bytes()).hexdigest())
