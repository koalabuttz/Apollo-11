#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Lossless whole-image actual original mixed INITDSP source stages."""
import pathlib,subprocess,hashlib,json,gzip
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build';E=R/'deps/virtualagc/yaAGC';F=B/'fixed-data/Comanche055';original=R/'evidence/fixed-data-cm-original.rope.bin'
subprocess.run(['python3','tools/cm_fixed_compat_generate.py'],cwd=R,check=True)
assert (F/'MAIN.agc.bin').read_bytes()==original.read_bytes()
assert hashlib.sha256(original.read_bytes()).hexdigest()=='6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061'
subprocess.run(['cc','-std=c99','-O2','-Iinclude','-Itests','-I'+str(E),'tools/cm_init_display_mixed_reference.c','tests/cm_init_display_mixed_cases.c','tests/cm_priority_complete_cases.c','tests/cm_context_change_cases.c','tests/cm_admission_complete_cases.c','src/cm_native_image.c','src/agc_word.c','tests/cm_executive_cases.c','src/cm_executive.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-init-display-mixed-reference'],cwd=R,check=True)
for role,cmd in [('native',['./build/cm-init-display-mixed-native']),('reference',['./build/cm-init-display-mixed-reference',str(original)])]:
 h=hashlib.sha256();count=0
 with (B/f'cm-init-display-mixed-{role}.snapshots.bin.gz').open('wb') as f,(B/f'cm-init-display-mixed-{role}.trace').open('w') as e:
  with gzip.GzipFile(fileobj=f,filename='',mode='wb',mtime=0,compresslevel=3) as out:
   p=subprocess.Popen(cmd,cwd=R,stdout=subprocess.PIPE,stderr=e)
   while True:
    data=p.stdout.read(1024*1024)
    if not data:break
    h.update(data);count+=len(data);out.write(data)
   assert p.wait()==0,role+' runner failed'
 (B/f'cm-init-display-mixed-{role}.stream.json').write_text(json.dumps({'bytes':count,'sha256':h.hexdigest()})+'\n')
a=json.loads((B/'cm-init-display-mixed-reference.stream.json').read_text());b=json.loads((B/'cm-init-display-mixed-native.stream.json').read_text());assert a==b,(a,b);assert a['bytes']==15360*4108
proof={'cases':7168,'checkpoints':15360,'full_physical_words':2048,'raw_registers':['A','L','Q'],'original_layout_rope_sha256':hashlib.sha256(original.read_bytes()).hexdigest(),'entries':['INITDSP03261','PRIOCHNG05210','INITDSP+6 03267'],'context_cohorts':'current or all6 selectedcores × basic/interpreter incoming ×512 priority/raw-register/bank/OVFIND/PUSH/restore cases','binary_bytes':a['bytes'],'snapshot_sha256':a['sha256'],'binary_record_format':'2054littleendian uint16: case low/high,entry,status,2048 physical words,SUPERBNK,IRQ','scope':'actual INITDSP prefix, mixed priority and actual current3267 return; selected basic/interpreter application body execution remains separate'}
(B/'cm-init-display-mixed-proof.json').write_text(json.dumps(proof,indent=2)+'\n');print('PASS: original mixed INITDSP source phases with all selected context types');print(json.dumps(proof,indent=2))
