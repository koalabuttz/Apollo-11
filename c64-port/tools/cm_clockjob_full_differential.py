#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,hashlib,sys
root=pathlib.Path(__file__).resolve().parents[1];engine=root/'deps/virtualagc/yaAGC'
def rows(p):return [' '.join(x.split('#',1)[0].split())for x in p.read_text().splitlines()if x.split('#',1)[0].strip()]
for f in ['P40-P47.agc','INTER-BANK_COMMUNICATION.agc']:
 a=rows(root.parent/'Comanche055'/f);b=rows(root/'deps/cm-services-fixture'/f)
 first,last=('CLOCKJOB ','E7SETTER ')if f=='P40-P47.agc'else('BANKCALL ','POSTJUMP ')
 a=a[next(i for i,x in enumerate(a)if x.startswith(first)):next(i for i,x in enumerate(a)if x.startswith(last))]
 b=b[next(i for i,x in enumerate(b)if x.startswith(first)):next(i for i,x in enumerate(b)if x.startswith(last))]
 assert a==b,f
base=['gcc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests']
subprocess.run(base+['src/cm_clockjob.c','src/cm_quictrig.c','src/agc_multiword.c','src/agc_word.c','tests/cm_trig_cases.c','tests/cm_clockjob_cases.c','tools/cm_clockjob_full_snapshot.c','-o','build/cm-clockjob-full-native'],cwd=root,check=True)
subprocess.run(['gcc','-std=c99','-O2','-ffunction-sections','-Wl,--gc-sections','-Iinclude','-Itests','-I'+str(engine),'src/agc_word.c','tests/cm_trig_cases.c','tests/cm_clockjob_cases.c','tools/cm_clockjob_full_reference.c','tests/scenarios.c',str(engine/'agc_engine.o'),str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/cm-clockjob-full-reference'],cwd=root,check=True)
output=[]
for name,cmd in [('native',['build/cm-clockjob-full-native']),('reference',['build/cm-clockjob-full-reference','deps/cm-services-fixture/MAIN.agc.bin'])]:
 p=subprocess.run(cmd,cwd=root,capture_output=True,text=True,check=True);(root/f'build/cm-clockjob-full-{name}.snapshots').write_text(p.stdout);output.append(p.stdout.splitlines())
assert len(output[0])==len(output[1])==2048
bad=[(a,b)for a,b in zip(*output)if a!=b]
for a,b in bad[:12]:
 av=list(map(int,a.split()));bv=list(map(int,b.split()));print('DIFF',av[0],[(oct(i-2),oct(x),oct(y))for i,(x,y)in enumerate(zip(av,bv))if x!=y])
if bad:sys.exit(f'{len(bad)} differing states')
print('PASS:2048 complete2048-word actual CM CLOCKJOB composed bridge/QUICTRIG checkpoints, plus SUPER/IRQ')
print('Snapshot SHA256:',hashlib.sha256((root/'build/cm-clockjob-full-reference.snapshots').read_bytes()).hexdigest())
