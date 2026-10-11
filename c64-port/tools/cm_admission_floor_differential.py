#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Lossless original Comanche Executive zero-candidate source states."""
import pathlib,subprocess,hashlib,json,gzip
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build';E=R/'deps/virtualagc/yaAGC';F=B/'fixed-data/Comanche055';original=R/'evidence/fixed-data-cm-original.rope.bin'
subprocess.run(['python3','tools/prepare_cm_original_rope.py'],cwd=R,check=True)
assert (F/'MAIN.agc.bin').read_bytes()==original.read_bytes()
assert hashlib.sha256(original.read_bytes()).hexdigest()=='6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061'
subprocess.run(['cc','-std=c99','-O2','-Iinclude','-Itests','-I'+str(E),'tools/cm_admission_floor_reference.c','tests/cm_admission_floor_cases.c','tests/cm_admission_complete_cases.c','tests/cm_executive_cases.c','src/cm_native_image.c','src/agc_word.c','src/cm_executive.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-admission-floor-reference'],cwd=R,check=True)
for role,cmd in [('native',['./build/cm-admission-floor-native']),('reference',['./build/cm-admission-floor-reference',str(original)])]:
 h=hashlib.sha256();count=0
 with (B/f'cm-admission-floor-{role}.snapshots.bin.gz').open('wb') as f,(B/f'cm-admission-floor-{role}.trace').open('w') as e:
  with gzip.GzipFile(fileobj=f,filename='',mode='wb',mtime=0,compresslevel=3) as out:
   p=subprocess.Popen(cmd,cwd=R,stdout=subprocess.PIPE,stderr=e)
   while True:
    data=p.stdout.read(1024*1024)
    if not data:break
    h.update(data);count+=len(data);out.write(data)
   assert p.wait()==0,role+' runner failed'
 (B/f'cm-admission-floor-{role}.stream.json').write_text(json.dumps({'bytes':count,'sha256':h.hexdigest()})+'\n')
a=json.loads((B/'cm-admission-floor-reference.stream.json').read_text());b=json.loads((B/'cm-admission-floor-native.stream.json').read_text());assert a==b,(a,b);assert a['bytes']==49152*4130
proof={'cases':49152,'checkpoints':49152,'full_physical_words':2048,'external_latches':13,'binary_bytes':a['bytes'],'snapshot_sha256':a['sha256'],'original_layout_rope_sha256':hashlib.sha256(original.read_bytes()).hexdigest(),'entries':['NOVAC','FINDVAC','SPVAC','SPVACIN'],'scope':'source bare priority+0 admission becomes positive packed FAKEPRET/VAC resource priority, all core+0 unavailable positions/resource outcomes; actual external job or BAILOUT handoff'}
with gzip.open(B/'cm-admission-floor-reference.snapshots.bin.gz','rb') as source,gzip.open(B/'cm-admission-floor-native.snapshots.bin.gz','rb') as native:
 while True:
  x,y=source.read(4130),native.read(4130);assert x==y
  if not x:break

(B/'cm-admission-floor-proof.json').write_text(json.dumps(proof,indent=2)+'\n');print('PASS: original bare priority+0 admission paths, complete image and all13 external metadata fields');print(json.dumps(proof,indent=2))
