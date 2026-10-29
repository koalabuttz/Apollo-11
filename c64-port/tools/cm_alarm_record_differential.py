#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Lossless whole-image actual original mixed INITDSP source stages."""
import pathlib,subprocess,hashlib,json,gzip
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build';E=R/'deps/virtualagc/yaAGC';F=B/'fixed-data/Comanche055';original=R/'evidence/fixed-data-cm-original.rope.bin'
subprocess.run(['python3','tools/cm_fixed_compat_generate.py'],cwd=R,check=True)
assert (F/'MAIN.agc.bin').read_bytes()==original.read_bytes()
assert hashlib.sha256(original.read_bytes()).hexdigest()=='6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061'
subprocess.run(['cc','-std=c99','-O2','-Iinclude','-Itests','-I'+str(E),'tools/cm_alarm_record_reference.c','tests/cm_alarm_record_cases.c','tests/cm_admission_complete_cases.c','src/cm_native_image.c','src/agc_word.c','tests/cm_executive_cases.c','src/cm_executive.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-alarm-record-reference'],cwd=R,check=True)
for role,cmd in [('native',['./build/cm-alarm-record-native']),('reference',['./build/cm-alarm-record-reference',str(original)])]:
 h=hashlib.sha256();count=0
 with (B/f'cm-alarm-record-{role}.snapshots.bin.gz').open('wb') as f,(B/f'cm-alarm-record-{role}.trace').open('w') as e:
  with gzip.GzipFile(fileobj=f,filename='',mode='wb',mtime=0,compresslevel=3) as out:
   p=subprocess.Popen(cmd,cwd=R,stdout=subprocess.PIPE,stderr=e)
   while True:
    data=p.stdout.read(1024*1024)
    if not data:break
    h.update(data);count+=len(data);out.write(data)
   assert p.wait()==0,role+' runner failed'
 (B/f'cm-alarm-record-{role}.stream.json').write_text(json.dumps({'bytes':count,'sha256':h.hexdigest()})+'\n')
a=json.loads((B/'cm-alarm-record-reference.stream.json').read_text());b=json.loads((B/'cm-alarm-record-native.stream.json').read_text());assert a==b,(a,b);assert a['bytes']==16384*4108
proof={'cases':16384,'checkpoints':16384,'full_physical_words':2048,'raw_registers':['A','L','Q'],'original_layout_rope_sha256':hashlib.sha256(original.read_bytes()).hexdigest(),'entries':['ALARM05650','VARALARM05764','CURTAINS05757','BORTENT05655'],'context_cohorts':'four entries × all signedzero/positive/negative FAILREG corner combinations, alarm codes and bank/lamp restoration states','binary_bytes':a['bytes'],'snapshot_sha256':a['sha256'],'binary_record_format':'2054littleendian uint16: case low/high,entry,status,2048 physical words,SUPERBNK,IRQ','scope':'actual alarm record body and genuine ordinary caller return or MULTFAIL overflow POODOO entry; abort recovery body remains separate'}
(B/'cm-alarm-record-proof.json').write_text(json.dumps(proof,indent=2)+'\n');print('PASS: original alarm recording full image and actual return/POODOO frontier');print(json.dumps(proof,indent=2))
