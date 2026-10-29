#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import gzip,json,pathlib,struct,hashlib
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build'
proof=json.loads((B/'cm-wake-complete-proof.json').read_text())
inputs=[4096*slot+c for slot in range(8) for c in [0,1,2,3,5,7,255,4095]]
rows={};h=hashlib.sha256();count=0
with gzip.open(B/'cm-wake-complete-reference.snapshots.bin.gz','rb') as source,gzip.open(B/'cm-wake-complete-native.snapshots.bin.gz','rb') as native:
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
s='/* SPDX-License-Identifier: MIT; original source corpus '+proof['snapshot_sha256']+' */\n#ifndef CM_WAKE_ZERO_VECTORS_H\n#define CM_WAKE_ZERO_VECTORS_H\n#include <stdint.h>\n#define CM_WAKE_TARGET_CASES 64u\n'
s+='static const uint32_t cm_wake_indices[64]={'+','.join(str(n)+'UL' for n in inputs)+'};\n#ifndef CM_WAKE_INDEX_ONLY\n'
s+='static const uint32_t cm_wake_hash[64]={'+','.join(str(fnv(rows[n]))+'UL' for n in inputs)+'};\n'
s+='static const uint16_t cm_wake_samples[2][2065]={'+','.join('{'+','.join(map(str,rows[inputs[i]]))+'}' for i in [0,63])+'};\n#endif\n#endif\n'
(R/'tests/cm_wake_complete_vectors.h').write_text(s)
(B/'cm-wake-complete-final-source-sample.bin').write_bytes(struct.pack('<2048H',*rows[inputs[-1]][4:2052]))
print('PASS: source-derived64 wake/core0/exception transfer C64 cases')
