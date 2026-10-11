#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import gzip,json,pathlib,struct,hashlib
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build'
proof=json.loads((B/'cm-priority-display-proof.json').read_text())
inputs=[0,1,7,11,23,31,63,127,255,256,504,511,767,1023,1272,1279,1535,1791,2047,2303,2559,2815,3071,3583,4095,4607,5119,5631,6143,7167,7679,8191]
rows={n:[] for n in inputs};h=hashlib.sha256();count=0
with gzip.open(B/'cm-priority-display-reference.snapshots.bin.gz','rb') as f,gzip.open(B/'cm-priority-display-native.snapshots.bin.gz','rb') as g:
 while True:
  a,b=f.read(4130),g.read(4130);assert a==b
  if not a:break
  assert len(a)==4130;h.update(a);count+=1;v=struct.unpack('<2065H',a);n=v[0]|(v[1]<<16)
  if n in rows:rows[n].append(v)
assert count==22362 and h.hexdigest()==proof['snapshot_sha256'];assert all(len(rows[n]) in [2,3] for n in inputs)
def fnv(v):
 h=2166136261
 for x in v:
  h=((h^(x&255))*16777619)&0xffffffff;h=((h^(x>>8))*16777619)&0xffffffff
 return h
s='/* SPDX-License-Identifier: MIT; original source stream '+h.hexdigest()+' */\n#ifndef CM_PRIORITY_DISPLAY_VECTORS_H\n#define CM_PRIORITY_DISPLAY_VECTORS_H\n#include <stdint.h>\n#define CM_PRIORITY_DISPLAY_TARGET_CASES 32u\n'
s+='static const uint32_t cm_priority_display_indices[32]={'+','.join(str(n)+'UL' for n in inputs)+'};\n'
s+='static const uint8_t cm_priority_display_counts[32]={'+','.join(str(len(rows[n])) for n in inputs)+'};\n#ifndef CM_PRIORITY_DISPLAY_INDEX_ONLY\n'
s+='static const uint32_t cm_priority_display_hash[32][3]={'+','.join('{'+','.join(str(fnv(v))+'UL' for v in rows[n])+'}' for n in inputs)+'};\n'
s+='static const uint16_t cm_priority_display_samples[2][2065]={'+','.join('{'+','.join(map(str,v))+'}' for v in [rows[inputs[0]][0],rows[inputs[-1]][-1]])+'};\n#endif\n#endif\n'
(R/'tests/cm_priority_display_vectors.h').write_text(s)
(B/'cm-priority-display-final-source-sample.bin').write_bytes(struct.pack('<2048H',*rows[inputs[-1]][-1][4:2052]))
(B/'cm-priority-display-target-counts.json').write_text(json.dumps({'cases':32,'checkpoints':sum(map(len,rows.values())),'lengths':[len(rows[n]) for n in inputs]})+'\n')
print('PASS:32 representative uninterrupted source paths, source admission failures and immediate caller returns')
