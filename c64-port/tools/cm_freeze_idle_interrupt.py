#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import gzip,json,pathlib,struct,hashlib
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build'
proof=json.loads((B/'cm-idle-interrupt-proof.json').read_text())
inputs=[2048*entry+c for entry in range(2) for c in [0,1,2,3,4,5,6,7,8,15,16,31,32,63,64,127,128,255,256,511,512,767,768,1023,1024,1279,1280,1535,1536,1791,1792,2047]]
rows={};h=hashlib.sha256();count=0
with gzip.open(B/'cm-idle-interrupt-reference.snapshots.bin.gz','rb') as source,gzip.open(B/'cm-idle-interrupt-native.snapshots.bin.gz','rb') as native:
 while True:
  a,b=source.read(4134),native.read(4134);assert a==b
  if not a:break
  assert len(a)==4134;h.update(a);count+=1;r=struct.unpack('<2067H',a);n=r[0]|r[1]<<16
  if n in inputs:rows[n]=r
assert count==4096 and h.hexdigest()==proof['snapshot_sha256'] and len(rows)==64
def fnv(row):
 h=2166136261
 for v in row:h=((h^(v&255))*16777619)&0xffffffff;h=((h^(v>>8))*16777619)&0xffffffff
 return h
s='/* SPDX-License-Identifier: MIT; original source corpus '+proof['snapshot_sha256']+' */\n#ifndef CM_IDLE_ZERO_VECTORS_H\n#define CM_IDLE_ZERO_VECTORS_H\n#include <stdint.h>\n#define CM_IDLE_TARGET_CASES 64u\n'
s+='static const uint32_t cm_idle_indices[64]={'+','.join(str(n)+'UL' for n in inputs)+'};\n#ifndef CM_IDLE_INDEX_ONLY\n'
s+='static const uint32_t cm_idle_hash[64]={'+','.join(str(fnv(rows[n]))+'UL' for n in inputs)+'};\n'
s+='static const uint16_t cm_idle_samples[2][2067]={'+','.join('{'+','.join(map(str,rows[inputs[i]]))+'}' for i in [0,63])+'};\n#endif\n#endif\n'
(R/'tests/cm_idle_interrupt_vectors.h').write_text(s)
(B/'cm-idle-interrupt-final-source-sample.bin').write_bytes(struct.pack('<2048H',*rows[inputs[-1]][4:2052]))
print('PASS: source-derived64 wake/core0/exception transfer C64 cases')
