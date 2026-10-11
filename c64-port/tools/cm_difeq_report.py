#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Audit full physical CM DIFEQ images and reproducibly compress the evidence."""
import pathlib,hashlib,json,gzip,shutil,re
root=pathlib.Path(__file__).resolve().parents[1];build=root/'build';digest='9b8b544ee7eac53ad9ac77fa6f27e4d980a23b0e3dcf22d7843e9ff4607a3ecf'
data=(build/'cm-difeq-reference.snapshots').read_bytes();assert len(data)==19840*2050*2 and hashlib.sha256(data).hexdigest()==digest
for name in ['native','sanitize']:assert data==(build/f'cm-difeq-{name}.snapshots').read_bytes()
loc=[[0o3447,0o3626,0o3630,0o3633,0o3634,0o3636,0o3641,0o3643,0o3644],[0o3453,0o3455,0o3456,0o3460,0o3626,0o3630,0o3633,0o3634,0o3636,0o3641,0o3643,0o3644],[0o3465,0o3467,0o3471,0o3474,0o3476,0o3500,0o3501,0o3503,0o3504]]
report={'scope':'Original ORBITAL_INTEGRATION DIFEQ+0/+1 including DIFEQCOM through STORE ALPHAV; DIFEQ+2 through STORE ZV. This is not the complete orbital integrator or complete Comanche application.','binary_format':'Little-endian uint16 case, phase, complete2048-word physical erasable image; phase255 is input.','source_audit':json.loads((build/'cm-orbital-source-audit.json').read_text()),'calls':1792,'original_stage_calls':1536,'additional_direct_index_corners':256,'checkpoints':19840,'complete_state_words':2048,'sha256':digest,'input_constructor_sha256':hashlib.sha256((root/'tests/cm_difeq_cases.c').read_bytes()).hexdigest(),'sanitizers':'Entire full-image corpus ASAN/UBSAN equals unchanged original source','c64':[]}
for stage,count in enumerate([9,12,9]):
 for phase in range(count):
  selector=stage*16+phase;tag=f'cm-difeq_selector{selector}';mail=(build/f'{tag}-vice-mailbox.bin').read_bytes();assert mail==bytes([0xa5,0,0,32,selector]);mapping=(build/f'cm-difeq-selector{selector}.map').read_text();end=0
  for name in ['.text','.rodata','.data','.bss','.noinit']:
   values=re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+([0-9a-f]+)\s+\d+\s+'+re.escape(name)+r'$',mapping,re.M);assert len(values)==1;end=max(end,int(values[0][0],16)+int(values[0][1],16))
  assert end<=0xc000
  for name in ['.zp.data','.zp.bss']:
   values=re.findall(r'^\s*[0-9a-f]+\s+[0-9a-f]+\s+([0-9a-f]+)\s+\d+\s+'+re.escape(name)+r'$',mapping,re.M);assert len(values)==1 and int(values[0],16)==0
  stack={}
  for name,base,top in [('hardware',0x100,0x1f2),('software',0xc100,0xd000)]:
   raw=(build/f'{tag}-{name}-stack.bin').read_bytes();changed=[i for i,v in enumerate(raw)if v!=0xcd];extent=top-base-min(changed)if changed else 0;assert extent<len(raw);stack[name]=extent
  internal=(stage==0 and phase==1)or(stage==1 and phase==4)
  report['c64'].append({'stage':stage,'phase':phase,'selector':selector,'complete_entry_exit_pairs':32,'mailbox':mail.hex(),'image_end':f'{end:04x}','stack_extent_bytes':stack,'source_loc':f'{loc[stage][phase]:05o}','source_pc':'06037'if internal else'06030','executive_check_allowed':not internal})
for name in ['native','reference','sanitize']:
 src=build/f'cm-difeq-{name}.snapshots';dst=src.with_suffix(src.suffix+'.gz')
 with src.open('rb')as f,dst.open('wb')as g:
  with gzip.GzipFile(filename='',mode='wb',fileobj=g,mtime=0,compresslevel=6)as z:shutil.copyfileobj(f,z)
 with gzip.open(dst,'rb')as z:assert hashlib.sha256(z.read()).hexdigest()==digest
report['compressed_snapshot_bytes']=(build/'cm-difeq-reference.snapshots.gz').stat().st_size
(build/'cm-difeq-proof.json').write_text(json.dumps(report,indent=2)+'\n')
print('PASS:19840 complete2048word original/native/sanitizer states plus960 complete C64 phase entry/exit states; build/cm-difeq-proof.json')
print('Maximum image end:',max(int(v['image_end'],16)for v in report['c64']))
print('Max stack:',{k:max(v['stack_extent_bytes'][k]for v in report['c64'])for k in ['hardware','software']})
print('Compressed corpus bytes:',report['compressed_snapshot_bytes'])
