#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,hashlib
R=pathlib.Path(__file__).resolve().parents[1];a=(R/'build/cm-mission-loop-reference.snapshots').read_bytes();assert a==(R/'build/cm-mission-loop-native.snapshots').read_bytes()
rows=[list(map(int,line.split()))for line in a.decode().splitlines()];assert len(rows)==2624 and all(len(r)==2059 for r in rows)
def fnv(v):
 h=2166136261
 for x in v:
  for b in [x&255,x>>8]:h=((h^b)*16777619)&0xffffffff
 return h
groups=[[r for r in rows if r[0]==n]for n in range(384)];indices=[(j%6)*64+(j//6)*5 for j in range(64)]
s='/* SPDX-License-Identifier: MIT; original corpus SHA256 '+hashlib.sha256(a).hexdigest()+' */\n#ifndef CM_MISSION_LOOP_VECTORS_H\n#define CM_MISSION_LOOP_VECTORS_H\n#include <stdint.h>\nstatic const uint8_t cm_mission_loop_count[384]={'+','.join(str(len(g))for g in groups)+'};\n#ifdef __mos__\nstatic const uint16_t cm_mission_loop_indices[64]={'+','.join(map(str,indices))+'};\nstatic const uint32_t cm_mission_loop_hash[64][10]={\n'
s+=',\n'.join('{'+','.join(str(fnv(r[2:]))+'u'for r in groups[n])+'}'for n in indices)+'};\n#else\nstatic const uint32_t cm_mission_loop_hash[384][10]={\n'+',\n'.join('{'+','.join(str(fnv(r[2:]))+'u'for r in g)+'}'for g in groups)+'};\n#endif\nstatic const uint16_t cm_mission_loop_sample[2057]={'+','.join(map(str,groups[0][-1][2:]))+'};\n#endif\n'
(R/'tests/cm_mission_loop_vectors.h').write_text(s)
s='/* SPDX-License-Identifier: MIT; original continuous source state proof */\n#ifndef CM_MISSION_OVERLAY_VECTORS_H\n#define CM_MISSION_OVERLAY_VECTORS_H\n#include <stdint.h>\nstatic const uint8_t cm_mission_overlay_count[6]={'+','.join(str(len(groups[n*64]))for n in range(6))+'};\nstatic const uint32_t cm_mission_overlay_hash[6][10]={'+','.join('{'+','.join(str(fnv(r[2:]))+'u'for r in groups[n*64])+'}'for n in range(6))+'};\nstatic const uint16_t cm_mission_overlay_sample[2057]={'+','.join(map(str,groups[0][-1][2:]))+'};\n#endif\n';(R/'tests/cm_mission_overlay_vectors.h').write_text(s)
print('384 continuous cases/2624 complete source checkpoints frozen')
