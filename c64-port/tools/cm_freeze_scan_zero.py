#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import gzip,json,pathlib,struct,hashlib
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build'
proof=json.loads((B/'cm-scan-zero-proof.json').read_text())
inputs=[4096*k+6*c+z for k in range(3) for c in [0,7,127,681] for z in range(6)]
rows={};h=hashlib.sha256();count=0
with gzip.open(B/'cm-scan-zero-reference.snapshots.bin.gz','rb') as source,gzip.open(B/'cm-scan-zero-native.snapshots.bin.gz','rb') as native:
 while True:
  a,b=source.read(4130),native.read(4130);assert a==b
  if not a:break
  assert len(a)==4130;h.update(a);count+=1;r=struct.unpack('<2065H',a);n=r[0]|r[1]<<16
  if n in inputs:rows[n]=r
assert count==12288 and h.hexdigest()==proof['snapshot_sha256'] and len(rows)==72
def fnv(row):
 h=2166136261
 for v in row:h=((h^(v&255))*16777619)&0xffffffff;h=((h^(v>>8))*16777619)&0xffffffff
 return h
s='/* SPDX-License-Identifier: MIT; original source corpus '+proof['snapshot_sha256']+' */\n#ifndef CM_SCAN_ZERO_VECTORS_H\n#define CM_SCAN_ZERO_VECTORS_H\n#include <stdint.h>\n#define CM_SCAN_TARGET_CASES 72u\n'
s+='static const uint32_t cm_scan_indices[72]={'+','.join(str(n)+'UL' for n in inputs)+'};\n#ifndef CM_SCAN_INDEX_ONLY\n'
s+='static const uint32_t cm_scan_hash[72]={'+','.join(str(fnv(rows[n]))+'UL' for n in inputs)+'};\n'
s+='static const uint16_t cm_scan_samples[2][2065]={'+','.join('{'+','.join(map(str,rows[inputs[i]]))+'}' for i in [0,71])+'};\n#endif\n#endif\n'
(R/'tests/cm_scan_zero_vectors.h').write_text(s)
(B/'cm-scan-zero-final-source-sample.bin').write_bytes(struct.pack('<2048H',*rows[inputs[-1]][4:2052]))
print('PASS: source-derived72 three-entry/all-six-zero-slot C64 cases')
