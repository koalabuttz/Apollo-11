#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Lossless actual original PRIOLARM and PRIOENT+1 source states."""
import pathlib,subprocess,hashlib,json,gzip
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build';E=R/'deps/virtualagc/yaAGC';F=B/'fixed-data/Comanche055';original=R/'evidence/fixed-data-cm-original.rope.bin'
subprocess.run(['python3','tools/cm_fixed_compat_generate.py'],cwd=R,check=True)
assert (F/'MAIN.agc.bin').read_bytes()==original.read_bytes()
assert hashlib.sha256(original.read_bytes()).hexdigest()=='6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061'
subprocess.run(['cc','-std=c99','-O2','-Iinclude','-Itests','-I'+str(E),'tools/cm_alarm_complete_reference.c','tests/cm_alarm_complete_cases.c','tests/cm_alarm_image_cases.c','tests/cm_alarm_record_cases.c','tests/cm_admission_complete_cases.c','src/cm_native_image.c','src/agc_word.c','tests/cm_executive_cases.c','src/cm_executive.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-alarm-complete-reference'],cwd=R,check=True)
for role,cmd in [('native',['./build/cm-alarm-complete-native']),('reference',['./build/cm-alarm-complete-reference',str(original)])]:
 h=hashlib.sha256();count=0
 with (B/f'cm-alarm-complete-{role}.snapshots.bin.gz').open('wb') as f,(B/f'cm-alarm-complete-{role}.trace').open('w') as e:
  with gzip.GzipFile(fileobj=f,filename='',mode='wb',mtime=0,compresslevel=3) as out:
   p=subprocess.Popen(cmd,cwd=R,stdout=subprocess.PIPE,stderr=e)
   while True:
    data=p.stdout.read(1024*1024)
    if not data:break
    h.update(data);count+=len(data);out.write(data)
   assert p.wait()==0,role+' runner failed'
 (B/f'cm-alarm-complete-{role}.stream.json').write_text(json.dumps({'bytes':count,'sha256':h.hexdigest()})+'\n')
a=json.loads((B/'cm-alarm-complete-reference.stream.json').read_text());b=json.loads((B/'cm-alarm-complete-native.stream.json').read_text());assert a==b,(a,b);assert a['bytes']==32240*4130
proof={'cases':32240,'checkpoints':32240,'full_physical_words':2048,'original_layout_rope_sha256':hashlib.sha256(original.read_bytes()).hexdigest(),'binary_bytes':a['bytes'],'snapshot_sha256':a['sha256'],'source_in_isr_invariance':'original engine exercised both source ISR contexts, asserted unchanged per case; native wrapper does not own or write caller ISR metadata','scope':'actual four ordinary alarm entries: everyQ0100..7775×all4entries×both virtualIRQ contexts, post-store inlinealiases, FB/SUPER/EB/FAILREGcohorts. Additive BORT normalreturn05721 IRQ1 versus genuineoverflowPOODOO IRQpreservation, no frozen source changes.'}
with gzip.open(B/'cm-alarm-complete-reference.snapshots.bin.gz','rb') as source,gzip.open(B/'cm-alarm-complete-native.snapshots.bin.gz','rb') as native:
 while True:
  x,y=source.read(4130),native.read(4130);assert x==y
  if not x:break

(B/'cm-alarm-complete-proof.json').write_text(json.dumps(proof,indent=2)+'\n');print('PASS: original ALARM caller-data aliases complete image and all13 external latches');print(json.dumps(proof,indent=2))
