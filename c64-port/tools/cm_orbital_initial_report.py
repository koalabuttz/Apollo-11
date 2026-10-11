#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Audit whole-image CM rectification proof and write reproducible compressed evidence."""
import pathlib,hashlib,json,gzip,shutil,re
root=pathlib.Path(__file__).resolve().parents[1];build=root/'build';digest='5a0688f511a9c6097a5aeafe9b8ac628c6ff9598345b029c40e0471e0d1cb37c'
data=(build/'cm-orbital-initial-reference.snapshots').read_bytes();assert len(data)==5120*2051*2 and hashlib.sha256(data).hexdigest()==digest
for name in ['native','sanitize']:assert data==(build/f'cm-orbital-initial-{name}.snapshots').read_bytes()
report={'scope':'ORBITAL_INTEGRATION DIFEQ0 initialization and JSWITCH branch to ACCOMP/DOW, four explicit source phases; not the complete orbital integrator or Comanche application.','binary_format':'Little-endian uint16 case, phase, complete2048-word erasable image; IRQ latch follows the image; phase255 is input, phases0..3 source returns.','source_audit':json.loads((build/'cm-orbital-source-audit.json').read_text()),'cases':1024,'checkpoints':5120,'complete_state_words':2048,'external_state_words':1,'sha256':digest,'input_constructor_sha256':hashlib.sha256((root/'tests/cm_orbital_initial_cases.c').read_bytes()).hexdigest(),'sanitizers':'Entire complete-image corpus ASAN/UBSAN equals unchanged source','c64':[]}
for phase in range(4):
 tag=f'cm-orbital-initial_phase{phase}';mail=(build/f'{tag}-vice-mailbox.bin').read_bytes();assert mail==bytes([0xa5,0,0,32,phase]);mapping=(build/f'cm-orbital-initial-phase{phase}.map').read_text();end=0
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
 src=build/f'cm-orbital-initial-{name}.snapshots';dst=src.with_suffix(src.suffix+'.gz')
 with src.open('rb')as f,dst.open('wb')as g:
  with gzip.GzipFile(filename='',mode='wb',fileobj=g,mtime=0,compresslevel=6)as z:shutil.copyfileobj(f,z)
 with gzip.open(dst,'rb')as z:assert hashlib.sha256(z.read()).hexdigest()==digest
report['compressed_snapshot_bytes']=(build/'cm-orbital-initial-reference.snapshots.gz').stat().st_size
(build/'cm-orbital-initial-proof.json').write_text(json.dumps(report,indent=2)+'\n')
print('PASS:5120 complete2048word+IRQ source/native/sanitizer states plus128 complete C64 source entry/exit states; build/cm-orbital-initial-proof.json')
print('Compressed complete-state corpus bytes:',report['compressed_snapshot_bytes'])
