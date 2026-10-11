#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,json,hashlib
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build'
a=(B/'cm-clock-display-mission-reference.snapshots').read_bytes();assert a==(B/'cm-clock-display-mission-native.snapshots').read_bytes()
p=json.loads((B/'cm-clock-display-mission-proof.json').read_text());assert hashlib.sha256(a).hexdigest()==p['snapshot_sha256']
rows=[list(map(int,l.split()))for l in a.decode().splitlines()];assert len(rows)==1344 and all(len(r)==2060 for r in rows)
indices=[0,15,16,31,32,47,48,63];groups=[[r for r in rows if r[0]==n]for n in indices]
assert all(g[-1][9]==0o3312 for g in groups[:4]);assert all(g[-1][9]==0o6037 for g in groups[4:])
assert not any(r[2]==103 for r in rows), 'This cohort does not claim busy END'
def fnv(v):
 h=2166136261
 for x in v:
  for b in [x&255,x>>8]:h=((h^b)*16777619)&0xffffffff
 return h
s='/* SPDX-License-Identifier: MIT; original continuous source state proof */\n#ifndef CM_CLOCK_DISPLAY_OVERLAY_VECTORS_H\n#define CM_CLOCK_DISPLAY_OVERLAY_VECTORS_H\n#include <stdint.h>\nstatic const uint16_t cm_clock_display_overlay_indices[8]={'+','.join(map(str,indices))+'};\nstatic const uint8_t cm_clock_display_overlay_count[8]={'+','.join(str(len(g))for g in groups)+'};\nstatic const uint32_t cm_clock_display_overlay_hash[8][40]={'+','.join('{'+','.join(str(fnv(r[2:]))+'u'for r in g)+'}'for g in groups)+'};\nstatic const uint16_t cm_clock_display_overlay_sample[3][2058]={'+','.join('{'+','.join(map(str,groups[n][-1][2:]))+'}'for n in [0,2,7])+'};\n#endif\n'
(R/'tests/cm_clock_display_overlay_vectors.h').write_text(s)
print('Frozen64 actual source sequences; normal GOPLAY and sleep/restored interpreter; no busy END claim')
print('Target',2*sum(len(g)for g in groups),'source checkpoints')
