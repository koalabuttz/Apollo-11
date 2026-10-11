#!/usr/bin/env python3
"""Freeze CM complete-state samples/fingerprints after exact source agreement."""
import hashlib,pathlib
R=pathlib.Path(__file__).resolve().parents[1];p=R/'build/cm-restart-reference.snapshots';assert p.read_bytes()==(R/'build/cm-restart-native.snapshots').read_bytes();rows=[list(map(int,r.split())) for r in p.read_text().splitlines()];assert len(rows)==1280 and all(len(r)==431 for r in rows)
indices=[0,1,255,256,257,511,512,513,767,768,769,780,792,793,804,806,816,1279]
s='/* SPDX-License-Identifier: MIT\n * GENERATED from original CM assembly by tools/cm_freeze_restart.py.\n * Snapshot SHA256: '+hashlib.sha256(p.read_bytes()).hexdigest()+' */\n#ifndef CM_RESTART_VECTORS_H\n#define CM_RESTART_VECTORS_H\n#include <stdint.h>\nstatic const uint32_t cm_rs_hashes[1280]={\n'
for r in rows:
 h=2166136261
 for w in r[1:]:
  for b in [w&255,(w>>8)&255]:h=((h^b)*16777619)&0xffffffff
 s+=' UINT32_C(0x%08x),\n'%h
s+='};\n#define CM_RS_FULL_COUNT18 18\nstatic const uint16_t cm_rs_indices[18]={'+','.join(str(i) for i in indices)+'};\nstatic const uint16_t cm_rs_full[18][430]={\n'+''.join(' {'+','.join(str(w) for w in rows[i][1:])+'},\n' for i in indices)+'};\n#endif\n'
(R/'tests/cm_restart_vectors.h').write_text(s)
print('Frozen1280CMrestart hashes +18complete430-word boundary states')
