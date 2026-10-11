#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,hashlib,json
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build';F=R/'deps/cm-finish-fixture';E=R/'deps/virtualagc/yaAGC'
subprocess.run(['python3','tools/cm_finish_prepare_reference.py'],cwd=R,check=True)
assert (F/'INTERPRETER.agc.original').read_bytes()==(R.parent/'Comanche055/INTERPRETER.agc').read_bytes();assert (F/'EXECUTIVE.agc').read_bytes()==(R/'deps/cm-services-fixture/EXECUTIVE.agc').read_bytes()
subprocess.run(['cc','-std=c99','-O2','-Iinclude','-Itests','-I'+str(E),'tools/cm_danzig_reference.c','tests/cm_danzig_cases.c','tests/cm_basic_finish_cases.c','src/agc_word.c','tests/cm_executive_cases.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-danzig-reference'],cwd=R,check=True)
for role,cmd in [('native',['./build/cm-danzig-native']),('reference',['./build/cm-danzig-reference',str(F/'MAIN.agc.bin')])]:
 with(B/f'cm-danzig-{role}.snapshots').open('w')as o,(B/f'cm-danzig-{role}.trace').open('w')as e:subprocess.run(cmd,cwd=R,stdout=o,stderr=e,check=True)
a=(B/'cm-danzig-reference.snapshots').read_bytes();b=(B/'cm-danzig-native.snapshots').read_bytes()
if a!=b:
 for x,y in zip(a.decode().splitlines(),b.decode().splitlines()):
  if x!=y:
   xv=list(map(int,x.split()));yv=list(map(int,y.split()));print('DIFF',xv[0],[(oct(i-2),oct(xx),oct(yy))for i,(xx,yy)in enumerate(zip(xv,yv))if xx!=yy]);break
 raise AssertionError('Full CM DANZIG states differ')
rows=a.decode().splitlines();assert len(rows)==4096 and all(len(r.split())==2056 for r in rows)
proof={'cases':4096,'full_physical_words':2048,'raw_registers':['A','L','Q'],'entry':'06030 DANZIG','boundaries':['selected basic DTCB','06037 NEWOPS after interpreted INTRSM','06037 ordinary continuation'],'external_words':['SUPERBNK','IRQ','channel11'],'selected_native_continuation':'actual basic/interpreted pair','snapshot_sha256':hashlib.sha256(a).hexdigest()}
(B/'cm-danzig-proof.json').write_text(json.dumps(proof,indent=2)+'\n');print('PASS: 4096 original CM full-image DANZIG basic/interpreted/check states');print(json.dumps(proof,indent=2))
