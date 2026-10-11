#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Lossless actual original PRIOLARM and PRIOENT+1 source states."""
import pathlib,subprocess,hashlib,json,gzip
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build';E=R/'deps/virtualagc/yaAGC';F=B/'fixed-data/Comanche055';original=R/'evidence/fixed-data-cm-original.rope.bin'
subprocess.run(['python3','tools/cm_fixed_compat_generate.py'],cwd=R,check=True)
assert (F/'MAIN.agc.bin').read_bytes()==original.read_bytes()
assert hashlib.sha256(original.read_bytes()).hexdigest()=='6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061'
subprocess.run(['cc','-std=c99','-O2','-Iinclude','-Itests','-I'+str(E),'tools/cm_priority_display_reference.c','tests/cm_priority_display_cases.c','tests/cm_admission_complete_cases.c','src/cm_native_image.c','src/agc_word.c','tests/cm_executive_cases.c','src/cm_executive.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-priority-display-reference'],cwd=R,check=True)
for role,cmd in [('native',['./build/cm-priority-display-native']),('reference',['./build/cm-priority-display-reference',str(original)])]:
 h=hashlib.sha256();count=0
 with (B/f'cm-priority-display-{role}.snapshots.bin.gz').open('wb') as f,(B/f'cm-priority-display-{role}.trace').open('w') as e:
  with gzip.GzipFile(fileobj=f,filename='',mode='wb',mtime=0,compresslevel=3) as out:
   p=subprocess.Popen(cmd,cwd=R,stdout=subprocess.PIPE,stderr=e)
   while True:
    data=p.stdout.read(1024*1024)
    if not data:break
    h.update(data);count+=len(data);out.write(data)
   assert p.wait()==0,role+' runner failed'
 (B/f'cm-priority-display-{role}.stream.json').write_text(json.dumps({'bytes':count,'sha256':h.hexdigest()})+'\n')
a=json.loads((B/'cm-priority-display-reference.stream.json').read_text());b=json.loads((B/'cm-priority-display-native.stream.json').read_text());assert a==b,(a,b);assert a['bytes']==22362*4130
with gzip.open(B/'cm-priority-display-reference.snapshots.bin.gz','rb') as source,gzip.open(B/'cm-priority-display-native.snapshots.bin.gz','rb') as native:
 while True:
  x,y=source.read(4130),native.read(4130)
  assert x==y, 'complete original image differs'
  if not x:break
proof={'cases':8192,'checkpoints':22362,'full_physical_words':2048,'raw_registers':['A','L','Q'],'original_layout_rope_sha256':hashlib.sha256(original.read_bytes()).hexdigest(),'entries':['PRIODSPR10,2726→SPVAC5160','genuine SPVAC→BOTHJOBS10,3107 or BAILOUT7755','BOTHJOBS→SAVELOCS/SAVELOCR→BANKJUMP4704→actual caller'],'context_cohorts':'all seven core scan positions, fiveVAC/full resource exhaustion, +0 unavailable occupied cores, allEB/SUPER/physicalFB savedcaller groups, savedcaller descriptor corners and actual source carry/TSoverflow, source NEWJOB ownership; all13 external latches compared','binary_bytes':a['bytes'],'snapshot_sha256':a['sha256'],'binary_record_format':'2065littleendian uint16: case low/high,phase,status,2048physicalwords,13 cm_native_io latches','scope':'actual flashing priority-display immediate-return control; queued MAKEPLAY and abort recovery bodies remain separately qualified; admission alarm metadata is read from original selected inline literal'}
(B/'cm-priority-display-proof.json').write_text(json.dumps(proof,indent=2)+'\n');print('PASS: uninterrupted original PRIODSPR admission/caller return, complete image and all13 external latches');print(json.dumps(proof,indent=2))
