#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Lossless whole-image actual original DSPFMEM/COMADRS/SUPDACAL proof."""
import pathlib,subprocess,hashlib,json,gzip
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build';E=R/'deps/virtualagc/yaAGC';F=B/'fixed-data/Comanche055';original=R/'evidence/fixed-data-cm-original.rope.bin'
subprocess.run(['python3','tools/cm_fixed_compat_generate.py'],cwd=R,check=True)
assert (F/'MAIN.agc.bin').read_bytes()==original.read_bytes()
assert hashlib.sha256(original.read_bytes()).hexdigest()=='6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061'
subprocess.run(['cc','-std=c99','-O2','-Iinclude','-Itests','-I'+str(E),'tools/cm_admission_complete_reference.c','tests/cm_admission_complete_cases.c','src/cm_native_image.c','src/agc_word.c','tests/cm_executive_cases.c','src/cm_executive.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-admission-complete-reference'],cwd=R,check=True)
for role,cmd in [('native',['./build/cm-admission-complete-native']),('reference',['./build/cm-admission-complete-reference',str(original)])]:
 h=hashlib.sha256();count=0
 with (B/f'cm-admission-complete-{role}.snapshots.bin.gz').open('wb') as f,(B/f'cm-admission-complete-{role}.trace').open('w') as e:
  with gzip.GzipFile(fileobj=f,filename='',mode='wb',mtime=0,compresslevel=3) as out:
   p=subprocess.Popen(cmd,cwd=R,stdout=subprocess.PIPE,stderr=e)
   while True:
    data=p.stdout.read(1024*1024)
    if not data:break
    h.update(data);count+=len(data);out.write(data)
   assert p.wait()==0,role+' runner failed'
 (B/f'cm-admission-complete-{role}.stream.json').write_text(json.dumps({'bytes':count,'sha256':h.hexdigest()})+'\n')
a=json.loads((B/'cm-admission-complete-reference.stream.json').read_text());b=json.loads((B/'cm-admission-complete-native.stream.json').read_text());assert a==b,(a,b);assert a['bytes']==12288*4108
proof={'cases':12288,'checkpoints':12288,'full_physical_words':2048,'raw_registers':['A','L','Q'],'original_layout_rope_sha256':hashlib.sha256(original.read_bytes()).hexdigest(),'entries':['NOVAC05134','FINDVAC05147','SPVAC05160','SPVACIN05154'],'resource_cohorts':'all8 first-free-core outcomes including exhaustion × all6 first-free-VAC outcomes including exhaustion,64 banks/priorities/background sets','binary_bytes':a['bytes'],'snapshot_sha256':a['sha256'],'binary_record_format':'2054littleendian uint16: case low/high,entry,status,2048 physical words,SUPERBNK,IRQ','scope':'successful source returns and exact BAILOUT entry with resource reservations preserved; no alarm-body execution; caller-valid positive priority and ordinary source addresses'}
(B/'cm-admission-complete-proof.json').write_text(json.dumps(proof,indent=2)+'\n');print('PASS: original canonical admission/resource failures and SPVAC entries');print(json.dumps(proof,indent=2))
