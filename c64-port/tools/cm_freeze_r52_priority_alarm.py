#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import gzip,json,pathlib,struct,hashlib
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build'
proof=json.loads((B/'cm-r52-priority-alarm-proof.json').read_text())
inputs=[0,1,2,3,4,5,6,7,8,15,23,31,39,47,55,63,64,127,255,511,767,1023,1279,1535,1791,2015,2023,2031,2039,2045,2046,2047]
rows={n:[] for n in inputs};h=hashlib.sha256();count=0
with gzip.open(B/'cm-r52-priority-alarm-reference.snapshots.bin.gz','rb') as f,gzip.open(B/'cm-r52-priority-alarm-native.snapshots.bin.gz','rb') as g:
 while True:
  a,b=f.read(4130),g.read(4130);assert a==b
  if not a:break
  assert len(a)==4130;h.update(a);count+=1;v=struct.unpack('<2065H',a);n=v[0]|(v[1]<<16)
  if n in rows:rows[n].append(v)
assert count==12407 and h.hexdigest()==proof['snapshot_sha256'];assert all(len(rows[n]) in [4,7] for n in inputs)
def fnv(v):
 h=2166136261
 for x in v:
  h=((h^(x&255))*16777619)&0xffffffff;h=((h^(x>>8))*16777619)&0xffffffff
 return h
s='/* SPDX-License-Identifier: MIT; original source stream '+h.hexdigest()+' */\n#ifndef CM_R52_PRIORITY_ALARM_VECTORS_H\n#define CM_R52_PRIORITY_ALARM_VECTORS_H\n#include <stdint.h>\n#define CM_R52_PRIORITY_ALARM_TARGET_CASES 32u\n'
s+='static const uint32_t cm_r52_priority_alarm_indices[32]={'+','.join(str(n)+'UL' for n in inputs)+'};\n'
s+='static const uint8_t cm_r52_priority_alarm_counts[32]={'+','.join(str(len(rows[n])) for n in inputs)+'};\n#ifndef CM_R52_PRIORITY_ALARM_INDEX_ONLY\n'
s+='static const uint32_t cm_r52_priority_alarm_hash[32][7]={'+','.join('{'+','.join(str(fnv(v))+'UL' for v in rows[n])+'}' for n in inputs)+'};\n'
s+='static const uint16_t cm_r52_priority_alarm_samples[2][2065]={'+','.join('{'+','.join(map(str,v))+'}' for v in [rows[inputs[0]][0],rows[inputs[-1]][-1]])+'};\n#endif\n#endif\n'
(R/'tests/cm_r52_priority_alarm_vectors.h').write_text(s)
(B/'cm-r52-priority-alarm-final-source-sample.bin').write_bytes(struct.pack('<2048H',*rows[inputs[-1]][-1][4:2052]))
(B/'cm-r52-priority-alarm-target-counts.json').write_text(json.dumps({'cases':32,'checkpoints':sum(map(len,rows.values())),'lengths':[len(rows[n]) for n in inputs]})+'\n')
print('PASS:32 representative uninterrupted source paths, source admission failures and immediate caller returns')
