#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,hashlib,json
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build';F=R/'deps/cm-finish-fixture';E=R/'deps/virtualagc/yaAGC'
subprocess.run(['python3','tools/cm_finish_prepare_reference.py'],cwd=R,check=True)
assert (F/'INTERPRETER.agc.original').read_bytes()==(R.parent/'Comanche055/INTERPRETER.agc').read_bytes();assert (F/'EXECUTIVE.agc').read_bytes()==(R/'deps/cm-services-fixture/EXECUTIVE.agc').read_bytes()
subprocess.run(['cc','-std=c99','-O2','-Iinclude','-Itests','-I'+str(E),'tools/cm_waitlist_extended_reference.c','tests/cm_waitlist_extended_cases.c','tests/cm_waitlist_image_cases.c','src/cm_native_image.c','src/cm_waitlist.c','src/agc_word.c','tests/cm_executive_cases.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-waitlist-extended-reference'],cwd=R,check=True)
for role,cmd in [('native',['./build/cm-waitlist-extended-native']),('reference',['./build/cm-waitlist-extended-reference',str(F/'MAIN.agc.bin')])]:
 with(B/f'cm-waitlist-extended-{role}.snapshots').open('w')as o,(B/f'cm-waitlist-extended-{role}.trace').open('w')as e:subprocess.run(cmd,cwd=R,stdout=o,stderr=e,check=True)
a=(B/'cm-waitlist-extended-reference.snapshots').read_bytes();b=(B/'cm-waitlist-extended-native.snapshots').read_bytes()
if a!=b:
 for x,y in zip(a.decode().splitlines(),b.decode().splitlines()):
  if x!=y:
   xv=list(map(int,x.split()));yv=list(map(int,y.split()));print('DIFF',xv[0],[(oct(i-2),oct(xx),oct(yy))for i,(xx,yy)in enumerate(zip(xv,yv))if xx!=yy]);break
 raise AssertionError('Full CM WAITLIST/TWIDDLE states differ')
rows=a.decode().splitlines();assert len(rows)==2048 and all(len(r.split())==2052 for r in rows)
proof={'cases':2048,'full_physical_words':2048,'raw_registers':['A','L','Q'],'entries':['05245 WAITLIST','05235 TWIDDLE'],'boundaries':['caller return','07755 BAILOUT','05721 POODOO'],'external_words':['SUPERBNK','IRQ'],'alarm_body':'later actual continuation, not executed by this helper','snapshot_sha256':hashlib.sha256(a).hexdigest()}
(B/'cm-waitlist-extended-proof.json').write_text(json.dumps(proof,indent=2)+'\n');print('PASS: 2048 original CM complete WAITLIST/TWIDDLE/alarm-entry states');print(json.dumps(proof,indent=2))
