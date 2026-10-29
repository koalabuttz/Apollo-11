#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Lossless original uninterrupted abort-to-ENEMA source lifecycle."""
import pathlib,subprocess,hashlib,json,gzip
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build';E=R/'deps/virtualagc/yaAGC';F=B/'fixed-data/Comanche055';original=R/'evidence/fixed-data-cm-original.rope.bin'
subprocess.run(['python3','tools/cm_fixed_compat_generate.py'],cwd=R,check=True)
assert (F/'MAIN.agc.bin').read_bytes()==original.read_bytes()
assert hashlib.sha256(original.read_bytes()).hexdigest()=='6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061'
subprocess.run(['cc','-std=c99','-O2','-DCM_RESTART_INPUT_ONLY','-Iinclude','-Itests','-I'+str(E),'tools/cm_abort_lifecycle_reference.c','tests/cm_abort_lifecycle_cases.c','tests/cm_enema_image_cases.c','tests/cm_enema_cases.c','tests/cm_restart_cases.c','src/cm_native_image.c','src/agc_word.c','tests/cm_executive_cases.c','src/cm_executive.c','src/cm_waitlist.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-abort-lifecycle-reference'],cwd=R,check=True)
for role,cmd in [('native',['./build/cm-abort-lifecycle-native']),('reference',['./build/cm-abort-lifecycle-reference',str(original)])]:
 h=hashlib.sha256();count=0
 with (B/f'cm-abort-lifecycle-{role}.snapshots.bin.gz').open('wb') as f,(B/f'cm-abort-lifecycle-{role}.trace').open('w') as e:
  with gzip.GzipFile(fileobj=f,filename='',mode='wb',mtime=0,compresslevel=3) as out:
   p=subprocess.Popen(cmd,cwd=R,stdout=subprocess.PIPE,stderr=e)
   while True:
    data=p.stdout.read(1024*1024)
    if not data:break
    h.update(data);count+=len(data);out.write(data)
   assert p.wait()==0,role+' runner failed'
 (B/f'cm-abort-lifecycle-{role}.stream.json').write_text(json.dumps({'bytes':count,'sha256':h.hexdigest()})+'\n')
a=json.loads((B/'cm-abort-lifecycle-reference.stream.json').read_text());b=json.loads((B/'cm-abort-lifecycle-native.stream.json').read_text());assert a==b,(a,b);assert a['bytes']==14848*4132
proof={'cases':1536,'checkpoints':14848,'full_physical_words':2048,'raw_registers':['A','L','Q'],'original_layout_rope_sha256':hashlib.sha256(original.read_bytes()).hexdigest(),'entries':['BAILOUT7755/POODOO5721/CCSHOLE5747','VAC5STOR→SWRETURN→BORTENT','V37FLBIT/threeDOWNFLAG/MR.KLEAN','WHIMPER→RESUME→POSTJUMP','ENEMA→actualMMDSPLAY/PTBAD/DOFSTART'],'context_cohorts':'512 legal original ENEMA input cohorts ×3 actualabort entries, source FAILREG+0 baseline and V37 both branches, actual Executive alarm producers, ISR0/1; uninterrupted7/8/14 checkpoint paths','binary_bytes':a['bytes'],'snapshot_sha256':a['sha256'],'binary_record_format':'2066littleendian uint16: case low/high,phase,status,2048 physical words,13 cm_native_io latches and ISR','scope':'actual uninterrupted abort snapshot/alarm/flag/phase/WHIMPER/ENEMA chain through genuine selected program frontier; following selected application body remains separate'}
(B/'cm-abort-lifecycle-proof.json').write_text(json.dumps(proof,indent=2)+'\n');print('PASS: original uninterrupted abort lifecycle, complete image/all13 latches/ISR');print(json.dumps(proof,indent=2))
