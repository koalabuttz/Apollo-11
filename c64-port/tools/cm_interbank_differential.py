#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,hashlib,json
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build';F=R/'deps/cm-interbank-fixture';E=R/'deps/virtualagc/yaAGC'
subprocess.run(['python3','tools/cm_interbank_prepare_reference.py'],cwd=R,check=True)
assert (F/'INTER-BANK_COMMUNICATION.agc.original').read_bytes()==(R.parent/'Comanche055/INTER-BANK_COMMUNICATION.agc').read_bytes();assert (F/'EXECUTIVE.agc').read_bytes()==(R/'deps/cm-services-fixture/EXECUTIVE.agc').read_bytes()
subprocess.run(['cc','-std=c99','-O2','-Iinclude','-Itests','-I'+str(E),'tools/cm_interbank_reference.c','tests/cm_interbank_cases.c','tests/cm_waitlist_image_cases.c','src/fixed_data.c','src/cm_native_image.c','src/cm_waitlist.c','src/agc_word.c','tests/cm_executive_cases.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-interbank-reference'],cwd=R,check=True)
for role,cmd in [('native',['./build/cm-interbank-native']),('reference',['./build/cm-interbank-reference',str(F/'MAIN.agc.bin')])]:
 with(B/f'cm-interbank-{role}.snapshots').open('w')as o,(B/f'cm-interbank-{role}.trace').open('w')as e:subprocess.run(cmd,cwd=R,stdout=o,stderr=e,check=True)
a=(B/'cm-interbank-reference.snapshots').read_bytes();b=(B/'cm-interbank-native.snapshots').read_bytes()
if a!=b:
 for x,y in zip(a.decode().splitlines(),b.decode().splitlines()):
  if x!=y:
   xv=list(map(int,x.split()));yv=list(map(int,y.split()));print('DIFF',xv[0],[(oct(i-2),oct(xx),oct(yy))for i,(xx,yy)in enumerate(zip(xv,yv))if xx!=yy]);break
 raise AssertionError('Full CM interbank states differ')
rows=a.decode().splitlines();assert len(rows)==7168 and all(len(r.split())==2052 for r in rows)
proof={'cases':7168,'full_physical_words':2048,'raw_registers':['A','L','Q'],'entries':['BANKCALL','SWCALL','SWRETURN','POSTJUMP','BANKJUMP','MAKECADR','SUPDACAL','IBNKCALL','ISWCALL','ISWRETRN','USPRCADR','SUPERSW','Q+10000','PRIO12'],'boundaries':['actual compiled target','actual source return'],'external_words':['SUPERBNK','IRQ'],'data_scope':'certified immutable fixed spans; all100 CM NNTYPTAB words directly exercised','snapshot_sha256':hashlib.sha256(a).hexdigest()}
(B/'cm-interbank-proof.json').write_text(json.dumps(proof,indent=2)+'\n');print('PASS: 7168 original CM complete interbank control/data states');print(json.dumps(proof,indent=2))
