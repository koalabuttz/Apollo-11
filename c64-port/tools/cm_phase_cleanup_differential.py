#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Lossless whole-image actual original mixed INITDSP source stages."""
import pathlib,subprocess,hashlib,json,gzip
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build';E=R/'deps/virtualagc/yaAGC';F=B/'fixed-data/Comanche055';original=R/'evidence/fixed-data-cm-original.rope.bin'
subprocess.run(['python3','tools/cm_fixed_compat_generate.py'],cwd=R,check=True)
assert (F/'MAIN.agc.bin').read_bytes()==original.read_bytes()
assert hashlib.sha256(original.read_bytes()).hexdigest()=='6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061'
subprocess.run(['cc','-std=c99','-O2','-Iinclude','-Itests','-I'+str(E),'tools/cm_phase_cleanup_reference.c','tests/cm_phase_cleanup_cases.c','tests/cm_vac5_store_cases.c','tests/cm_admission_complete_cases.c','src/cm_native_image.c','src/agc_word.c','tests/cm_executive_cases.c','src/cm_executive.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-phase-cleanup-reference'],cwd=R,check=True)
for role,cmd in [('native',['./build/cm-phase-cleanup-native']),('reference',['./build/cm-phase-cleanup-reference',str(original)])]:
 h=hashlib.sha256();count=0
 with (B/f'cm-phase-cleanup-{role}.snapshots.bin.gz').open('wb') as f,(B/f'cm-phase-cleanup-{role}.trace').open('w') as e:
  with gzip.GzipFile(fileobj=f,filename='',mode='wb',mtime=0,compresslevel=3) as out:
   p=subprocess.Popen(cmd,cwd=R,stdout=subprocess.PIPE,stderr=e)
   while True:
    data=p.stdout.read(1024*1024)
    if not data:break
    h.update(data);count+=len(data);out.write(data)
   assert p.wait()==0,role+' runner failed'
 (B/f'cm-phase-cleanup-{role}.stream.json').write_text(json.dumps({'bytes':count,'sha256':h.hexdigest()})+'\n')
a=json.loads((B/'cm-phase-cleanup-reference.stream.json').read_text());b=json.loads((B/'cm-phase-cleanup-native.stream.json').read_text());assert a==b,(a,b);assert a['bytes']==12288*4108
proof={'cases':12288,'checkpoints':12288,'full_physical_words':2048,'raw_registers':['A','L','Q'],'original_layout_rope_sha256':hashlib.sha256(original.read_bytes()).hexdigest(),'entries':['MR.KLEAN05,2501','P00KLEAN05,2505','V37KLEAN05,2513'],'context_cohorts':'three entries ×4096 randomized wholeimages, entry Ebank/SUPER/IRQ and rawregister domains','binary_bytes':a['bytes'],'snapshot_sha256':a['sha256'],'binary_record_format':'2054littleendian uint16: case low/high,entry,status,2048 physical words,SUPERBNK,IRQ','scope':'actual MR/P00/V37 cleanup, exact inactive phase(+0)/complement(-0) pairs and genuine ordinary return; selected recovery caller remains separate'}
(B/'cm-phase-cleanup-proof.json').write_text(json.dumps(proof,indent=2)+'\n');print('PASS: original source phase cleanup bodies and ordinary return');print(json.dumps(proof,indent=2))
