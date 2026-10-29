#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Lossless original Comanche Executive zero-candidate source states."""
import pathlib,subprocess,hashlib,json,gzip
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build';E=R/'deps/virtualagc/yaAGC';F=B/'fixed-data/Comanche055';original=R/'evidence/fixed-data-cm-original.rope.bin'
subprocess.run(['python3','tools/prepare_cm_original_rope.py'],cwd=R,check=True)
assert (F/'MAIN.agc.bin').read_bytes()==original.read_bytes()
assert hashlib.sha256(original.read_bytes()).hexdigest()=='6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061'
subprocess.run(['cc','-std=c99','-O2','-Iinclude','-Itests','-I'+str(E),'tools/cm_priority_full_reference.c','tests/cm_priority_full_cases.c','tests/cm_context_change_cases.c','tests/cm_admission_complete_cases.c','tests/cm_executive_cases.c','src/cm_native_image.c','src/agc_word.c','src/cm_executive.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-priority-full-reference'],cwd=R,check=True)
for role,cmd in [('native',['./build/cm-priority-full-native']),('reference',['./build/cm-priority-full-reference',str(original)])]:
 h=hashlib.sha256();count=0
 with (B/f'cm-priority-full-{role}.snapshots.bin.gz').open('wb') as f,(B/f'cm-priority-full-{role}.trace').open('w') as e:
  with gzip.GzipFile(fileobj=f,filename='',mode='wb',mtime=0,compresslevel=3) as out:
   p=subprocess.Popen(cmd,cwd=R,stdout=subprocess.PIPE,stderr=e)
   while True:
    data=p.stdout.read(1024*1024)
    if not data:break
    h.update(data);count+=len(data);out.write(data)
   assert p.wait()==0,role+' runner failed'
 (B/f'cm-priority-full-{role}.stream.json').write_text(json.dumps({'bytes':count,'sha256':h.hexdigest()})+'\n')
a=json.loads((B/'cm-priority-full-reference.stream.json').read_text());b=json.loads((B/'cm-priority-full-native.stream.json').read_text());assert a==b,(a,b);assert a['bytes']==75264*4136
proof={'cases':75264,'checkpoints':75264,'full_physical_words':2048,'external_latches':13,'binary_bytes':a['bytes'],'snapshot_sha256':a['sha256'],'original_layout_rope_sha256':hashlib.sha256(original.read_bytes()).hexdigest(),'typed_transfer_words':3,'scope':'actual PRIOCHNG all32 bare levels including+0, six authentic resource fractions, basic/interpreter/no exchange and every+0candidate position; true source caller or CCSHOLE transfer'}
with gzip.open(B/'cm-priority-full-reference.snapshots.bin.gz','rb') as source,gzip.open(B/'cm-priority-full-native.snapshots.bin.gz','rb') as native:
 while True:
  x,y=source.read(4136),native.read(4136);assert x==y
  if not x:break

(B/'cm-priority-full-proof.json').write_text(json.dumps(proof,indent=2)+'\n');print('PASS: original PRIOCHNG complete image, all13 metadata fields and actual typed transfer');print(json.dumps(proof,indent=2))
