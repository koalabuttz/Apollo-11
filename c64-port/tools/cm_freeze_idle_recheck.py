#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import gzip,json,pathlib,struct,hashlib
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build'
proof=json.loads((B/'cm-idle-recheck-proof.json').read_text())
inputs=[4096*k+c for k in range(3) for c in [0,1,2,3,4,5,6,7,31,32,127,255,511,767,1023,1373,1729,2047,2731,3071,4095]]+[8190]
rows={};h=hashlib.sha256();count=0
with gzip.open(B/'cm-idle-recheck-reference.snapshots.bin.gz','rb') as source,gzip.open(B/'cm-idle-recheck-native.snapshots.bin.gz','rb') as native:
 while True:
  a,b=source.read(4136),native.read(4136);assert a==b
  if not a:break
  assert len(a)==4136;h.update(a);count+=1;r=struct.unpack('<2068H',a);n=r[0]|r[1]<<16
  if n in inputs:rows[n]=r
assert count==12288 and h.hexdigest()==proof['snapshot_sha256'] and len(rows)==64
def fnv(row):
 h=2166136261
 for v in row:h=((h^(v&255))*16777619)&0xffffffff;h=((h^(v>>8))*16777619)&0xffffffff
 return h
s='/* SPDX-License-Identifier: MIT; original source corpus '+proof['snapshot_sha256']+' */\n#ifndef CM_IDLE_RECHECK_ZERO_VECTORS_H\n#define CM_IDLE_RECHECK_ZERO_VECTORS_H\n#include <stdint.h>\n#define CM_IDLE_RECHECK_TARGET_CASES 64u\n'
s+='static const uint32_t cm_idle_recheck_indices[64]={'+','.join(str(n)+'UL' for n in inputs)+'};\n#ifndef CM_IDLE_RECHECK_INDEX_ONLY\n'
s+='static const uint32_t cm_idle_recheck_hash[64]={'+','.join(str(fnv(rows[n]))+'UL' for n in inputs)+'};\n'
s+='static const uint16_t cm_idle_recheck_samples[2][2068]={'+','.join('{'+','.join(map(str,rows[inputs[i]]))+'}' for i in [0,63])+'};\n#endif\n#endif\n'
(R/'tests/cm_idle_recheck_vectors.h').write_text(s)
(B/'cm-idle-recheck-final-source-sample.bin').write_bytes(struct.pack('<2048H',*rows[inputs[-1]][4:2052]))
print('PASS: source-derived64 source-NUCHANG2/race+0/basic/interpreter C64 cases')
