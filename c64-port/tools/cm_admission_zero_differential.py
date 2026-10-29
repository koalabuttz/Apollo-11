#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Lossless original +0 core unavailable admission scans."""
import pathlib,subprocess,hashlib,json,gzip
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build';E=R/'deps/virtualagc/yaAGC';F=B/'fixed-data/Comanche055';original=R/'evidence/fixed-data-cm-original.rope.bin'
subprocess.run(['python3','tools/cm_fixed_compat_generate.py'],cwd=R,check=True)
assert (F/'MAIN.agc.bin').read_bytes()==original.read_bytes()
assert hashlib.sha256(original.read_bytes()).hexdigest()=='6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061'
subprocess.run(['cc','-std=c99','-O2','-Iinclude','-Itests','-I'+str(E),'tools/cm_admission_zero_reference.c','tests/cm_admission_zero_cases.c','tests/cm_admission_complete_cases.c','src/cm_native_image.c','src/agc_word.c','tests/cm_executive_cases.c','src/cm_executive.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-admission-zero-reference'],cwd=R,check=True)
for role,cmd in [('native',['./build/cm-admission-zero-native']),('reference',['./build/cm-admission-zero-reference',str(original)])]:
 h=hashlib.sha256();count=0
 with (B/f'cm-admission-zero-{role}.snapshots.bin.gz').open('wb') as f,(B/f'cm-admission-zero-{role}.trace').open('w') as e:
  with gzip.GzipFile(fileobj=f,filename='',mode='wb',mtime=0,compresslevel=3) as out:
   p=subprocess.Popen(cmd,cwd=R,stdout=subprocess.PIPE,stderr=e)
   while True:
    data=p.stdout.read(1024*1024)
    if not data:break
    h.update(data);count+=len(data);out.write(data)
   assert p.wait()==0,role+' runner failed'
 (B/f'cm-admission-zero-{role}.stream.json').write_text(json.dumps({'bytes':count,'sha256':h.hexdigest()})+'\n')
a=json.loads((B/'cm-admission-zero-reference.stream.json').read_text());b=json.loads((B/'cm-admission-zero-native.stream.json').read_text());assert a==b,(a,b);assert a['bytes']==86016*4108
proof={'cases':86016,'checkpoints':86016,'full_physical_words':2048,'raw_registers':['A','L','Q'],'original_layout_rope_sha256':hashlib.sha256(original.read_bytes()).hexdigest(),'entries':['NOVAC05134','FINDVAC05147','SPVAC05160','SPVACIN05154'],'resource_cohorts':'all7 positions overwritten with +0 × all8 prior first-free-core outcomes × all6 VAC outcomes ×64 source banks/priorities/background sets','binary_bytes':a['bytes'],'snapshot_sha256':a['sha256'],'binary_record_format':'2054littleendian uint16: case low/high,entry,status,2048 physical words,SUPERBNK,IRQ','scope':'defined CCS+0→numeric NO.CORES EXTEND→BZF NEXTCORE scan, genuine successful return or resource BAILOUT; ordinary compiled callers and legal underlying VAC/NEWJOB ownership contract retained'}
(B/'cm-admission-zero-proof.json').write_text(json.dumps(proof,indent=2)+'\n');print('PASS: original canonical admission/resource failures and SPVAC entries');print(json.dumps(proof,indent=2))
