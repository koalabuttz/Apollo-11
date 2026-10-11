#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import gzip,json,pathlib,struct,hashlib
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build'
proof=json.loads((B/'cm-super-dispatch-proof.json').read_text())
inputs=[2*c+entry for c in [0,1,7,8,15,16,31,32,63,64,127,128,255,256,511,512,1023,1024,2015,2016,2047,2048,4030,4031,4095,6143,8191,10239] for entry in range(2)]+[28672+c for c in [0,1,7,8,255,256,2047,4095]]
rows={};h=hashlib.sha256();count=0
with gzip.open(B/'cm-super-dispatch-reference.snapshots.bin.gz','rb') as source,gzip.open(B/'cm-super-dispatch-native.snapshots.bin.gz','rb') as native:
 while True:
  a,b=source.read(4130),native.read(4130);assert a==b
  if not a:break
  assert len(a)==4130;h.update(a);count+=1;r=struct.unpack('<2065H',a);n=r[0]|r[1]<<16
  if n in inputs:rows[n]=r
assert count==32768 and h.hexdigest()==proof['snapshot_sha256'] and len(rows)==64
def fnv(row):
 h=2166136261
 for v in row:h=((h^(v&255))*16777619)&0xffffffff;h=((h^(v>>8))*16777619)&0xffffffff
 return h
s='/* SPDX-License-Identifier: MIT; original source corpus '+proof['snapshot_sha256']+' */\n#ifndef CM_SUPER_ZERO_VECTORS_H\n#define CM_SUPER_ZERO_VECTORS_H\n#include <stdint.h>\n#define CM_SUPER_TARGET_CASES 64u\n'
s+='static const uint32_t cm_super_indices[64]={'+','.join(str(n)+'UL' for n in inputs)+'};\n#ifndef CM_SUPER_INDEX_ONLY\n'
s+='static const uint32_t cm_super_hash[64]={'+','.join(str(fnv(rows[n]))+'UL' for n in inputs)+'};\n'
s+='static const uint16_t cm_super_samples[2][2065]={'+','.join('{'+','.join(map(str,rows[inputs[i]]))+'}' for i in [0,63])+'};\n#endif\n#endif\n'
(R/'tests/cm_super_dispatch_vectors.h').write_text(s)
(B/'cm-super-dispatch-final-source-sample.bin').write_bytes(struct.pack('<2048H',*rows[inputs[-1]][4:2052]))
print('PASS: source-derived64 public superbank/P20 transfer C64 cases')
