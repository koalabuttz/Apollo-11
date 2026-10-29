#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Lossless actual original PRIOLARM and PRIOENT+1 source states."""
import pathlib,subprocess,hashlib,json,gzip
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build';E=R/'deps/virtualagc/yaAGC';F=B/'fixed-data/Comanche055';original=R/'evidence/fixed-data-cm-original.rope.bin'
subprocess.run(['python3','tools/cm_fixed_compat_generate.py'],cwd=R,check=True)
assert (F/'MAIN.agc.bin').read_bytes()==original.read_bytes()
assert hashlib.sha256(original.read_bytes()).hexdigest()=='6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061'
subprocess.run(['cc','-std=c99','-O2','-Iinclude','-Itests','-I'+str(E),'tools/cm_priolarm_reference.c','tests/cm_priolarm_cases.c','tests/cm_alarm_record_cases.c','tests/cm_admission_complete_cases.c','src/cm_native_image.c','src/agc_word.c','tests/cm_executive_cases.c','src/cm_executive.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-priolarm-reference'],cwd=R,check=True)
for role,cmd in [('native',['./build/cm-priolarm-native']),('reference',['./build/cm-priolarm-reference',str(original)])]:
 h=hashlib.sha256();count=0
 with (B/f'cm-priolarm-{role}.snapshots.bin.gz').open('wb') as f,(B/f'cm-priolarm-{role}.trace').open('w') as e:
  with gzip.GzipFile(fileobj=f,filename='',mode='wb',mtime=0,compresslevel=3) as out:
   p=subprocess.Popen(cmd,cwd=R,stdout=subprocess.PIPE,stderr=e)
   while True:
    data=p.stdout.read(1024*1024)
    if not data:break
    h.update(data);count+=len(data);out.write(data)
   assert p.wait()==0,role+' runner failed'
 (B/f'cm-priolarm-{role}.stream.json').write_text(json.dumps({'bytes':count,'sha256':h.hexdigest()})+'\n')
a=json.loads((B/'cm-priolarm-reference.stream.json').read_text());b=json.loads((B/'cm-priolarm-native.stream.json').read_text());assert a==b,(a,b);assert a['bytes']==65536*4130
proof={'cases':65536,'checkpoints':65536,'full_physical_words':2048,'raw_registers':['A','L','Q'],'original_layout_rope_sha256':hashlib.sha256(original.read_bytes()).hexdigest(),'entries':['PRIOLARM10,3766→PRIOENT+1','real PRIODSPR10,2726 or MULTFAIL→POODOO05721'],'context_cohorts':'every rawA16/rawQ16 entry value, all source FAILREG corner classes, original TSoverflow skip, all erasable banks, saved caller BBCON/SUPER and actual V05N09; thirteen external fields compared','binary_bytes':a['bytes'],'snapshot_sha256':a['sha256'],'binary_record_format':'2065littleendian uint16: case low/high,phase,status,2048 physical words,13 cm_native_io latches','scope':'actual PRIOLARM through genuine PRIODSPR or overflow POODOO entry; downstream display/recovery remains separate, raw TSoverflow may retain an invalid caller descriptor without fabricating a return'}
(B/'cm-priolarm-proof.json').write_text(json.dumps(proof,indent=2)+'\n');print('PASS: original PRIOLARM/PRIOENT+1 complete image and all13 external latches');print(json.dumps(proof,indent=2))
