#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import gzip,json,pathlib,struct,hashlib
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build';proof=json.loads((B/'cm-fixed-compat-proof.json').read_text());banks=[0,1,2,3,7,15,16,23,24,27,28,31,32,33,35,39];inputs=[banks[i%16]*1024+([0,1023,511,512][i%4])+(40960 if i>=16 else 0)for i in range(32)];selected=set(inputs);rows={n:[]for n in inputs};h=hashlib.sha256();count=0
with gzip.open(B/'cm-fixed-compat-reference.snapshots.bin.gz','rb')as source,gzip.open(B/'cm-fixed-compat-native.snapshots.bin.gz','rb')as native:
 while True:
  a=source.read(4108);b=native.read(4108);assert a==b
  if not a:break
  assert len(a)==4108;h.update(a);count+=1;r=struct.unpack('<2054H',a);n=r[0]|(r[1]<<16)
  if n in selected:rows[n].append(r)
assert h.hexdigest()==proof['snapshot_sha256'] and count==163840
assert all(len(rows[n])==2 and rows[n][0][2]==0 and rows[n][1][2]==1 for n in inputs)
def fnv(words):
 h=2166136261
 for x in words:
  h=((h^(x&255))*16777619)&0xffffffff;h=((h^(x>>8))*16777619)&0xffffffff
 return h
raw=(R/'evidence/fixed-data-cm-original.rope.bin').read_bytes();words=[]
for bank in range(40):
 if bank>=36:words.extend([0]*1024);continue
 index=bank^2 if bank<4 else bank;words.extend(int.from_bytes(raw[(index*1024+i)*2:(index*1024+i)*2+2],'big')>>1 for i in range(1024))
s='/* SPDX-License-Identifier: MIT; original complete corpus SHA256 '+proof['snapshot_sha256']+' */\n#ifndef CM_FIXED_COMPAT_VECTORS_H\n#define CM_FIXED_COMPAT_VECTORS_H\n#include <stdint.h>\n#define CM_FIXED_TARGET_CASES 32u\nstatic const uint32_t cm_fixed_compat_indices[32]={'+','.join(str(n)+'UL'for n in inputs)+'};\n#ifndef CM_FIXED_COMPAT_INDEX_ONLY\nstatic const uint32_t cm_fixed_compat_hash[32][2]={'+','.join('{'+','.join(str(fnv(r))+'UL'for r in rows[n])+'}' for n in inputs)+'};\nstatic const uint16_t cm_fixed_compat_samples[2][2054]={'+','.join('{'+','.join(map(str,r))+'}'for r in rows[inputs[0]])+'};\n#endif\n#define CM_FIXED_COMPAT_ALL_DATA_HASH '+str(fnv(words))+'UL\n#endif\n';(R/'tests/cm_fixed_compat_vectors.h').write_text(s)
(B/'cm-fixed-compat-final-source-sample.bin').write_bytes(struct.pack('<2048H',*rows[inputs[-1]][1][4:2052]));print('PASS: source-derived32 target actual caller inputs and64 complete state fingerprints; exhaustive40960-word passive data digest')
