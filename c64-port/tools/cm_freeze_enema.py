#!/usr/bin/env python3
"""Freeze original CM ENEMA full samples and complete-state fingerprints."""
import hashlib,pathlib
R=pathlib.Path(__file__).resolve().parents[1];p=R/'build/cm-enema-reference.snapshots';assert p.read_bytes()==(R/'build/cm-enema-native.snapshots').read_bytes();rows=[list(map(int,r.split())) for r in p.read_text().splitlines()];assert len(rows)==4096 and all(len(r)==441 for r in rows)
indices=[0,4,5,6,17,22,118,246]
s='/* SPDX-License-Identifier: MIT\n * GENERATED from original CM assembly by tools/cm_freeze_enema.py.\n * Snapshot SHA256: '+hashlib.sha256(p.read_bytes()).hexdigest()+' */\n#ifndef CM_ENEMA_VECTORS_H\n#define CM_ENEMA_VECTORS_H\n#include <stdint.h>\nstatic const uint32_t cm_enema_hashes[4096]={\n'
for r in rows:
 h=2166136261
 for w in r[1:]:
  for b in [w&255,(w>>8)&255]:h=((h^b)*16777619)&0xffffffff
 s+=' UINT32_C(0x%08x),\n'%h
s+='};\nstatic const uint16_t cm_enema_indices[8]={'+','.join(str(i) for i in indices)+'};\nstatic const uint16_t cm_enema_full[8][440]={\n'+''.join(' {'+','.join(str(w) for w in rows[i][1:])+'},\n' for i in indices)+'};\n#endif\n'
(R/'tests/cm_enema_vectors.h').write_text(s)
print('Frozen4096CMENEMA hashes+8complete440-word states')
