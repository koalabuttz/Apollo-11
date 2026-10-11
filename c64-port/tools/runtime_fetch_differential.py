#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,hashlib,sys
root=pathlib.Path(__file__).resolve().parents[1];engine=root/'deps/virtualagc/yaAGC'
def rows(p):return [' '.join(x.split('#',1)[0].split())for x in p.read_text().splitlines()if x.split('#',1)[0].strip()]
for f in ['INTERPRETER.agc','INTER-BANK_COMMUNICATION.agc']:
 a=rows(root.parent/'Luminary099'/f);b=rows(root/'deps/fixture'/f)
 if f=='INTER-BANK_COMMUNICATION.agc':
  a=a[next(i for i,x in enumerate(a)if x.startswith('BANKCALL ')):next(i for i,x in enumerate(a)if x.startswith('POSTJUMP '))];b=b[next(i for i,x in enumerate(b)if x.startswith('BANKCALL ')):next(i for i,x in enumerate(b)if x.startswith('POSTJUMP '))]
 assert a==b,f
base=['gcc','-std=c99','-O2','-Iinclude','-Itests']
subprocess.run(base+['src/agc_word.c','src/p66_runtime_bridge.c','src/p66_runtime_fetch_bridge.c','tests/throttle_bridge_cases.c','tests/display_control_cases.c','tests/runtime_bridge_cases.c','tests/runtime_fetch_cases.c','tools/runtime_fetch_snapshot.c','-o','build/runtime-fetch-native'],cwd=root,check=True)
subprocess.run(base+['-I'+str(engine),'src/agc_word.c','src/p66_runtime_bridge.c','tests/throttle_bridge_cases.c','tests/display_control_cases.c','tests/runtime_bridge_cases.c','tests/runtime_fetch_cases.c','tools/runtime_fetch_reference.c','tests/scenarios.c',str(engine/'agc_engine.o'),str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/runtime-fetch-reference'],cwd=root,check=True)
output=[]
for name,cmd in [('native',['build/runtime-fetch-native']),('reference',['build/runtime-fetch-reference','deps/fixture/MAIN.agc.bin'])]:
 p=subprocess.run(cmd,cwd=root,capture_output=True,text=True,check=True);(root/f'build/runtime-fetch-{name}.snapshots').write_text(p.stdout);output.append(p.stdout.splitlines())
assert len(output[0])==len(output[1])==8192
bad=[(a,b)for a,b in zip(*output)if a!=b]
for a,b in bad[:12]:
 av=list(map(int,a.split()));bv=list(map(int,b.split()));print('DIFF',av[0],[(oct(i-2),oct(x),oct(y))for i,(x,y)in enumerate(zip(av,bv))if x!=y])
if bad:sys.exit(f'{len(bad)} differing states')
print('PASS:8192 complete2048-word original source EXIT/BANKCALL and THDUMP/SWRETURN/INTPRET bridges, plus SUPER/IRQ')
print('Snapshot SHA256:',hashlib.sha256((root/'build/runtime-fetch-reference.snapshots').read_bytes()).hexdigest())
