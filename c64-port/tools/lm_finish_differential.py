#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,hashlib,sys
root=pathlib.Path(__file__).resolve().parents[1];engine=root/'deps/virtualagc/yaAGC'
def rows(p):return [' '.join(x.split('#',1)[0].split())for x in p.read_text().splitlines()if x.split('#',1)[0].strip()]
for f in ['INTERPRETER.agc','INTER-BANK_COMMUNICATION.agc','DISPLAY_INTERFACE_ROUTINES.agc','EXECUTIVE.agc']:
 a=rows(root.parent/'Luminary099'/f);b=rows(root/'deps/fixture'/f)
 if f=='INTER-BANK_COMMUNICATION.agc':
  a=a[next(i for i,x in enumerate(a)if x.startswith('BANKCALL ')):next(i for i,x in enumerate(a)if x.startswith('POSTJUMP '))];b=b[next(i for i,x in enumerate(b)if x.startswith('BANKCALL ')):next(i for i,x in enumerate(b)if x.startswith('POSTJUMP '))]
 if f=='DISPLAY_INTERFACE_ROUTINES.agc':
  # Fixture relocates this unusedalias declaration toavoidduplicate modernsymbol; no instructionschanged.
  assert [x for x in a if x!='PRIOSAVE EQUALS R1SAVE']==b,f
 else:assert a==b,f
base=['gcc','-std=c99','-O2','-Iinclude','-Itests']
subprocess.run(base+['src/lm_basic_finish.c','src/executive.c','src/agc_word.c','tests/throttle_bridge_cases.c','tests/lm_priority_cases.c','tests/lm_sleep_cases.c','tests/lm_finish_cases.c','tools/lm_finish_snapshot.c','-o','build/lm-finish-native'],cwd=root,check=True)
subprocess.run(base+['-I'+str(engine),'src/agc_word.c','tests/throttle_bridge_cases.c','tests/lm_priority_cases.c','tests/lm_sleep_cases.c','tests/lm_finish_cases.c','tools/lm_finish_reference.c','tests/scenarios.c',str(engine/'agc_engine.o'),str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/lm-finish-reference'],cwd=root,check=True)
output=[]
for name,cmd in [('native',['build/lm-finish-native']),('reference',['build/lm-finish-reference','deps/fixture/MAIN.agc.bin'])]:
 p=subprocess.run(cmd,cwd=root,capture_output=True,text=True,check=True);(root/f'build/lm-finish-{name}.snapshots').write_text(p.stdout);output.append(p.stdout.splitlines())
assert len(output[0])==len(output[1])==16384
bad=[(a,b)for a,b in zip(*output)if a!=b]
for a,b in bad[:12]:
 av=list(map(int,a.split()));bv=list(map(int,b.split()));print('DIFF',av[0],[(oct(i-2),oct(x),oct(y))for i,(x,y)in enumerate(zip(av,bv))if x!=y])
if bad:sys.exit(f'{len(bad)} differing states')
print('PASS:16384 complete2048-word original source LM ENDOFJOB/basic DTCB, interpreter INTRSM or actual ADVAN idle, plus SUPER/IRQ')
print('Snapshot SHA256:',hashlib.sha256((root/'build/lm-finish-reference.snapshots').read_bytes()).hexdigest())
