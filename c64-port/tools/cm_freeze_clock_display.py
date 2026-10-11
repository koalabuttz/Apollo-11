#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,json,hashlib
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build'
a=(B/'cm-clock-display-mission-reference.snapshots').read_bytes();assert a==(B/'cm-clock-display-mission-native.snapshots').read_bytes()
p=json.loads((B/'cm-clock-display-mission-proof.json').read_text());assert hashlib.sha256(a).hexdigest()==p['snapshot_sha256']
rows=[list(map(int,l.split()))for l in a.decode().splitlines()];assert len(rows)==1344 and all(len(r)==2060 for r in rows)
busy=(B/'cm-clock-display-busy-reference.snapshots').read_bytes();assert busy==(B/'cm-clock-display-busy-native.snapshots').read_bytes()
bp=json.loads((B/'cm-clock-display-busy-proof.json').read_text());assert hashlib.sha256(busy).hexdigest()==bp['snapshot_sha256']
brows=[list(map(int,l.split()))for l in busy.decode().splitlines()];assert len(brows)==1216 and sum(r[2]==103 for r in brows)==64
indices=[0,63,64];groups=[[r for r in rows if r[0]==0],[r for r in rows if r[0]==63],[r for r in brows if r[0]==0]]
assert groups[0][-1][9]==0o3312 and all(g[-1][9]==0o6037 for g in groups[1:])
def fnv(v):
 h=2166136261
 for x in v:
  for b in [x&255,x>>8]:h=((h^b)*16777619)&0xffffffff
 return h
s='/* SPDX-License-Identifier: MIT; original continuous source state proof */\n#ifndef CM_CLOCK_DISPLAY_OVERLAY_VECTORS_H\n#define CM_CLOCK_DISPLAY_OVERLAY_VECTORS_H\n#include <stdint.h>\nstatic const uint16_t cm_clock_display_overlay_indices[3]={'+','.join(map(str,indices))+'};\nstatic const uint8_t cm_clock_display_overlay_count[3]={'+','.join(str(len(g))for g in groups)+'};\nstatic const uint32_t cm_clock_display_overlay_hash[3][40]={'+','.join('{'+','.join(str(fnv(r[2:]))+'u'for r in g)+'}'for g in groups)+'};\nstatic const uint16_t cm_clock_display_overlay_sample[3][2058]={'+','.join('{'+','.join(map(str,groups[n][-1][2:]))+'}'for n in [0,1,2])+'};\n#endif\n'
(R/'tests/cm_clock_display_overlay_vectors.h').write_text(s)
print('Frozen128 actual source sequences; normal GOPLAY, sleep restoration and actual matching busy END; representative target3contexts bothI')
print('Target',2*sum(len(g)for g in groups),'source checkpoints')
