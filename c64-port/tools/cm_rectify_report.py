#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Audit whole-image CM rectification proof and write reproducible compressed evidence."""
import pathlib,hashlib,json,gzip,shutil,re
root=pathlib.Path(__file__).resolve().parents[1];build=root/'build';digest='7b93fc96fd7e845b9fec26c64cbfd48e96ba80dc472e1990cd21590381faee23'
data=(build/'cm-rectify-reference.snapshots').read_bytes();assert len(data)==24064*2050*2 and hashlib.sha256(data).hexdigest()==digest
for name in ['native','sanitize']:assert data==(build/f'cm-rectify-{name}.snapshots').read_bytes()
report={'scope':'ORBITAL_INTEGRATION RECTIFY/MINIRECT, twelve explicit source groups. This is not the complete Comanche application or complete orbital integrator.','binary_format':'Little-endian uint16 case, phase, complete2048-word erasable image; phase12 is independent input; phases0..11 are source returns.','source_audit':json.loads((build/'cm-orbital-source-audit.json').read_text()),'cases':2048,'checkpoints':24064,'complete_state_words':2048,'sha256':digest,'input_constructor_sha256':hashlib.sha256((root/'tests/cm_rectify_cases.c').read_bytes()).hexdigest(),'sanitizers':'Entire complete-image corpus ASAN/UBSAN equals unchanged source','c64':[]}
for phase in range(12):
 tag=f'cm-rectify_phase{phase}';mail=(build/f'{tag}-vice-mailbox.bin').read_bytes();assert mail==bytes([0xa5,0,0,32,phase]);mapping=(build/f'cm-rectify-phase{phase}.map').read_text();end=0
 for name in ['.text','.rodata','.data','.bss','.noinit']:
  values=re.findall(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+([0-9a-f]+)\s+\d+\s+'+re.escape(name)+r'$',mapping,re.M);assert len(values)==1;end=max(end,int(values[0][0],16)+int(values[0][1],16))
 assert end<=0xc000
 for name in ['.zp.data','.zp.bss']:
  values=re.findall(r'^\s*[0-9a-f]+\s+[0-9a-f]+\s+([0-9a-f]+)\s+\d+\s+'+re.escape(name)+r'$',mapping,re.M);assert len(values)==1 and int(values[0],16)==0
 stack={}
 for name,base,top in [('hardware',0x100,0x1f2),('software',0xc100,0xd000)]:
  raw=(build/f'{tag}-{name}-stack.bin').read_bytes();changed=[i for i,v in enumerate(raw)if v!=0xcd];extent=top-base-min(changed)if changed else 0;assert extent<len(raw);stack[name]=extent
 report['c64'].append({'phase':phase,'complete_entry_exit_pairs':32,'mailbox':mail.hex(),'image_end':f'{end:04x}','stack_extent_bytes':stack})
for name in ['native','reference','sanitize']:
 src=build/f'cm-rectify-{name}.snapshots';dst=src.with_suffix(src.suffix+'.gz')
 with src.open('rb')as f,dst.open('wb')as g:
  with gzip.GzipFile(filename='',mode='wb',fileobj=g,mtime=0,compresslevel=6)as z:shutil.copyfileobj(f,z)
 with gzip.open(dst,'rb')as z:assert hashlib.sha256(z.read()).hexdigest()==digest
report['compressed_snapshot_bytes']=(build/'cm-rectify-reference.snapshots.gz').stat().st_size
(build/'cm-rectify-proof.json').write_text(json.dumps(report,indent=2)+'\n')
print('PASS:24064 complete2048word source/native/sanitizer states plus384 complete C64 source entry/exit states; build/cm-rectify-proof.json')
print('Compressed complete-state corpus bytes:',report['compressed_snapshot_bytes'])
