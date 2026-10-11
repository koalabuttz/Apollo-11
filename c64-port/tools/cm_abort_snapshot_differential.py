#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Lossless whole-image actual original mixed INITDSP source stages."""
import pathlib,subprocess,hashlib,json,gzip
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build';E=R/'deps/virtualagc/yaAGC';F=B/'fixed-data/Comanche055';original=R/'evidence/fixed-data-cm-original.rope.bin'
subprocess.run(['python3','tools/cm_fixed_compat_generate.py'],cwd=R,check=True)
assert (F/'MAIN.agc.bin').read_bytes()==original.read_bytes()
assert hashlib.sha256(original.read_bytes()).hexdigest()=='6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061'
subprocess.run(['cc','-std=c99','-O2','-Iinclude','-Itests','-I'+str(E),'tools/cm_abort_snapshot_reference.c','tests/cm_abort_snapshot_cases.c','tests/cm_vac5_store_cases.c','tests/cm_admission_complete_cases.c','src/cm_native_image.c','src/agc_word.c','tests/cm_executive_cases.c','src/cm_executive.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-abort-snapshot-reference'],cwd=R,check=True)
for role,cmd in [('native',['./build/cm-abort-snapshot-native']),('reference',['./build/cm-abort-snapshot-reference',str(original)])]:
 h=hashlib.sha256();count=0
 with (B/f'cm-abort-snapshot-{role}.snapshots.bin.gz').open('wb') as f,(B/f'cm-abort-snapshot-{role}.trace').open('w') as e:
  with gzip.GzipFile(fileobj=f,filename='',mode='wb',mtime=0,compresslevel=3) as out:
   p=subprocess.Popen(cmd,cwd=R,stdout=subprocess.PIPE,stderr=e)
   while True:
    data=p.stdout.read(1024*1024)
    if not data:break
    h.update(data);count+=len(data);out.write(data)
   assert p.wait()==0,role+' runner failed'
 (B/f'cm-abort-snapshot-{role}.stream.json').write_text(json.dumps({'bytes':count,'sha256':h.hexdigest()})+'\n')
a=json.loads((B/'cm-abort-snapshot-reference.stream.json').read_text());b=json.loads((B/'cm-abort-snapshot-native.stream.json').read_text());assert a==b,(a,b);assert a['bytes']==61440*4108
proof={'cases':12288,'checkpoints':61440,'full_physical_words':2048,'raw_registers':['A','L','Q'],'original_layout_rope_sha256':hashlib.sha256(original.read_bytes()).hexdigest(),'entries':['BAILOUT07755','POODOO05721','CCSHOLE05747','VAC5STOR20,2000','SWRETURN04675','BORTENT05655'],'context_cohorts':'three genuine abort entry producers, two original Executive alarm words, 4096 randomized snapshot/phase/FAILREG/bank cases','binary_bytes':a['bytes'],'snapshot_sha256':a['sha256'],'binary_record_format':'2054littleendian uint16: case low/high,entry,status,2048 physical words,SUPERBNK,IRQ','scope':'actual abort entry through source snapshot and alarm record; stops at genuine WHIMPER/recovery branch entry'}
(B/'cm-abort-snapshot-proof.json').write_text(json.dumps(proof,indent=2)+'\n');print('PASS: original continuous abort snapshot and alarm record phases');print(json.dumps(proof,indent=2))
