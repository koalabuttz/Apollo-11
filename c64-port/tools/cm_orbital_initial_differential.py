#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,array,hashlib,sys
root=pathlib.Path(__file__).resolve().parents[1];v=root/'deps/virtualagc/yaAGC'
subprocess.run([sys.executable,'tools/cm_orbital_prepare_reference.py'],cwd=root,check=True)
base=['gcc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests']
core=['src/agc_word.c','src/cm_orbital_initial.c'];cases=['tests/cm_orbital_initial_cases.c']
subprocess.run(base+core+cases+['tools/cm_orbital_initial_snapshot.c','-o','build/cm-orbital-initial-native'],cwd=root,check=True)
subprocess.run(['gcc','-std=c99','-O2','-Iinclude','-Itests','-I'+str(v),'tools/cm_orbital_initial_reference.c']+cases+['tests/cm_executive_cases.c','src/agc_word.c',str(v/'agc_engine.o'),str(v/'agc_engine_init.o'),str(v/'rfopen.o'),'-o','build/cm-orbital-initial-reference'],cwd=root,check=True)
outputs=[]
for name,cmd in [('native',['build/cm-orbital-initial-native']),('reference',['build/cm-orbital-initial-reference','deps/cm-orbital-fixture/MAIN.agc.bin'])]:
 with(root/f'build/cm-orbital-initial-{name}.snapshots').open('wb')as o,(root/f'build/cm-orbital-initial-{name}.trace').open('w')as e:
  p=subprocess.run(cmd,cwd=root,stdout=o,stderr=e)
  if p.returncode:sys.exit(f'{name} failed; '+(root/f'build/cm-orbital-initial-{name}.trace').read_text()[-1200:])
 a=array.array('H');a.frombytes((root/f'build/cm-orbital-initial-{name}.snapshots').read_bytes());outputs.append(a)
assert len(outputs[0])==len(outputs[1]);errors=0
for k in range(0,len(outputs[0]),2051):
 a=outputs[0][k:k+2051];b=outputs[1][k:k+2051]
 if a!=b:
  errors+=1
  if errors<17:print('DIFF case/phase',list(a[:2]),[(oct(i),oct(x),oct(y))for i,(x,y)in enumerate(zip(a[2:],b[2:]))if x!=y][:30])
if errors:sys.exit(f'{errors} differing complete physical states')
print('PASS:1024 DIFEQ0 initialization calls;',len(outputs[0])//2051,'complete2048word+IRQ input/source-boundary states, both routes, raw corners and deterministic random corpus')
print('SHA256:',hashlib.sha256((root/'build/cm-orbital-initial-reference.snapshots').read_bytes()).hexdigest())
