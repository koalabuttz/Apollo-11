#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze eight complete original continuous dormant-core0 wake paths."""
import gzip,hashlib,json,pathlib,struct
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build'
proof=json.loads((B/'cm-idle-wake-proof.json').read_text());inputs=[0,9,18,27,60,125,254,511];rows={};h=hashlib.sha256();count=0
with gzip.open(B/'cm-idle-wake-reference.snapshots.bin.gz','rb') as a,gzip.open(B/'cm-idle-wake-native.snapshots.bin.gz','rb') as z:
 while True:
  x,y=a.read(4134),z.read(4134);assert x==y
  if not x:break
  h.update(x);count+=1;v=struct.unpack('<2067H',x);n=v[0]|v[1]<<16
  if n//9 in inputs:rows[n]=v
assert count==4608 and h.hexdigest()==proof['snapshot_sha256'] and len(rows)==72
def fnv(v):
 h=2166136261
 for n in v:h=((h^(n&255))*16777619)&0xffffffff;h=((h^(n>>8))*16777619)&0xffffffff
 return h
ordered=[n*9+p for n in inputs for p in range(9)]
s='/* SPDX-License-Identifier: MIT; original complete lifecycle stream '+proof['snapshot_sha256']+' */\n#ifndef CM_IDLE_WAKE_VECTORS_H\n#define CM_IDLE_WAKE_VECTORS_H\n#include <stdint.h>\nstatic const uint32_t cm_idle_wake_indices[8]={'+','.join(str(n)+'UL' for n in inputs)+'};\n#ifndef CM_IDLE_WAKE_INDEX_ONLY\nstatic const uint32_t cm_idle_wake_hash[72]={'+','.join(str(fnv(rows[n]))+'UL' for n in ordered)+'};\nstatic const uint16_t cm_idle_wake_samples[2][2067]={'+','.join('{'+','.join(map(str,rows[n]))+'}' for n in [ordered[0],ordered[-1]])+'};\n#endif\n#endif\n'
(R/'tests/cm_idle_wake_vectors.h').write_text(s);(B/'cm-idle-wake-final-source-sample.bin').write_bytes(struct.pack('<2048H',*rows[ordered[-1]][4:2052]));(B/'cm-idle-wake-target-indices.json').write_text(json.dumps({'source_case_ids':inputs,'checkpoint_ids':ordered,'source_stream_sha256':proof['snapshot_sha256']},indent=2)+'\n')
print('PASS:72 source checkpoints frozen from8 uninterrupted original nine-group paths; all delay-slot indexes0..7')
