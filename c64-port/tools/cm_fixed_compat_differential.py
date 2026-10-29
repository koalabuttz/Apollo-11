#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Lossless whole-image actual original DSPFMEM/COMADRS/SUPDACAL proof."""
import pathlib,subprocess,hashlib,json,gzip
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build';E=R/'deps/virtualagc/yaAGC';F=B/'fixed-data/Comanche055';original=R/'evidence/fixed-data-cm-original.rope.bin'
subprocess.run(['python3','tools/cm_fixed_compat_generate.py'],cwd=R,check=True)
assert (F/'MAIN.agc.bin').read_bytes()==original.read_bytes()
assert hashlib.sha256(original.read_bytes()).hexdigest()=='6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061'
subprocess.run(['cc','-std=c99','-O2','-Iinclude','-Itests','-I'+str(E),'tools/cm_fixed_compat_reference.c','tests/cm_fixed_compat_cases.c','src/agc_word.c','tests/cm_executive_cases.c','src/cm_executive.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-o','build/cm-fixed-compat-reference'],cwd=R,check=True)
for role,cmd in [('native',['./build/cm-fixed-compat-native']),('reference',['./build/cm-fixed-compat-reference',str(original)])]:
 h=hashlib.sha256();count=0
 with (B/f'cm-fixed-compat-{role}.snapshots.bin.gz').open('wb') as f,(B/f'cm-fixed-compat-{role}.trace').open('w') as e:
  with gzip.GzipFile(fileobj=f,filename='',mode='wb',mtime=0,compresslevel=3) as out:
   p=subprocess.Popen(cmd,cwd=R,stdout=subprocess.PIPE,stderr=e)
   while True:
    data=p.stdout.read(1024*1024)
    if not data:break
    h.update(data);count+=len(data);out.write(data)
   assert p.wait()==0,role+' runner failed'
 (B/f'cm-fixed-compat-{role}.stream.json').write_text(json.dumps({'bytes':count,'sha256':h.hexdigest()})+'\n')
a=json.loads((B/'cm-fixed-compat-reference.stream.json').read_text());b=json.loads((B/'cm-fixed-compat-native.stream.json').read_text());assert a==b,(a,b);assert a['bytes']==81920*2*4108
proof={'cases':81920,'checkpoints':163840,'full_physical_words':2048,'raw_registers':['A','L','Q'],'original_layout_rope_sha256':hashlib.sha256(original.read_bytes()).hexdigest(),'entries':['40960 actual DSPFMEM41,3353 entries','40960 actual COMADRS43,3535 entries'],'data_domain':'every physical original fixed word in banks0..35 plus all unpopulated bank36..39 zero words, both actual callers','boundaries':['actual SUPDACAL04715 entry','actual caller return3361 or3542 after original SUPDACAL'],'external_words':['SUPERBNK','IRQ'],'binary_record_format':'2054 little-endian uint16: case low/high,phase,status,2048 physical words,SUPERBNK,IRQ','binary_bytes':a['bytes'],'snapshot_sha256':a['sha256'],'native_instruction_decoder':False,'executable_coverage_claim':'caller lead-ins and SUPDACAL control only; fixed words remain data; later octal-display/checksum bodies not executed'}
(B/'cm-fixed-compat-proof.json').write_text(json.dumps(proof,indent=2)+'\n');print('PASS: original DSPFMEM/COMADRS/SUPDACAL complete original fixed-word domain');print(json.dumps(proof,indent=2))
