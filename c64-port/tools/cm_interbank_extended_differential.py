#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Lossless whole-image actual original DSPFMEM/COMADRS/SUPDACAL proof."""
import pathlib,subprocess,hashlib,json,gzip
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build';E=R/'deps/virtualagc/yaAGC';F=B/'fixed-data/Comanche055';original=R/'evidence/fixed-data-cm-original.rope.bin'
subprocess.run(['python3','tools/cm_fixed_compat_generate.py'],cwd=R,check=True)
assert (F/'MAIN.agc.bin').read_bytes()==original.read_bytes()
assert hashlib.sha256(original.read_bytes()).hexdigest()=='6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061'
subprocess.run(['cc','-std=c99','-O2','-Iinclude','-Itests','-I'+str(E),'tools/cm_interbank_extended_reference.c','tests/cm_fixed_compat_cases.c','tests/cm_interbank_extended_cases.c','src/agc_word.c','tests/cm_executive_cases.c','src/cm_executive.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-interbank-extended-reference'],cwd=R,check=True)
for role,cmd in [('native',['./build/cm-interbank-extended-native']),('reference',['./build/cm-interbank-extended-reference',str(original)])]:
 h=hashlib.sha256();count=0
 with (B/f'cm-interbank-extended-{role}.snapshots.bin.gz').open('wb') as f,(B/f'cm-interbank-extended-{role}.trace').open('w') as e:
  with gzip.GzipFile(fileobj=f,filename='',mode='wb',mtime=0,compresslevel=3) as out:
   p=subprocess.Popen(cmd,cwd=R,stdout=subprocess.PIPE,stderr=e)
   while True:
    data=p.stdout.read(1024*1024)
    if not data:break
    h.update(data);count+=len(data);out.write(data)
   assert p.wait()==0,role+' runner failed'
 (B/f'cm-interbank-extended-{role}.stream.json').write_text(json.dumps({'bytes':count,'sha256':h.hexdigest()})+'\n')
a=json.loads((B/'cm-interbank-extended-reference.stream.json').read_text());b=json.loads((B/'cm-interbank-extended-native.stream.json').read_text());assert a==b,(a,b);assert a['bytes']==917504*4108
proof={'cases':917504,'checkpoints':917504,'actual_source_executed_cases':901120,'rejected_invalid_BANKJUMP_negative_overflow':16384,'full_physical_words':2048,'raw_registers':['A','L','Q'],'original_layout_rope_sha256':hashlib.sha256(original.read_bytes()).hexdigest(),'entries':14,'raw_A_inputs_per_entry':65536,'binary_bytes':a['bytes'],'snapshot_sha256':a['sha256'],'binary_record_format':'2054littleendian uint16: case low/high,entry,status,2048 physical words,SUPERBNK,IRQ','scope':'all raw16 A input preflights for every control group, 901120 executed original source boundaries, 16384 negative-overflow BANKJUMP invalid typed transfers explicitly rejected before mutation; rawL bijective variation; ordinary compiled Q0705; passive original words only; no decoder'}
(B/'cm-interbank-extended-proof.json').write_text(json.dumps(proof,indent=2)+'\n');print('PASS: original extended interbank rawA branch states');print(json.dumps(proof,indent=2))
