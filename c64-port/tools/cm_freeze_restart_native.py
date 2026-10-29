#!/usr/bin/env python3
"""Freeze source-executed CM restart/job callback boundary evidence."""
import pathlib,hashlib
R=pathlib.Path(__file__).resolve().parents[1];p=R/'build/cm-restart-native-reference.snapshots'
assert p.read_bytes()==(R/'build/cm-restart-native-native.snapshots').read_bytes()
rows=[list(map(int,r.split())) for r in p.read_text().splitlines()]
assert len(rows)==320 and all(len(r)==436 for r in rows)
indices=[0,1,2,3,4,9,118,319]
s='/* SPDX-License-Identifier: MIT\n * GENERATED from original source restart/Executive and explicit callback\n * caller fixtures. Snapshot SHA256: '+hashlib.sha256(p.read_bytes()).hexdigest()+' */\n#ifndef CM_RESTART_NATIVE_VECTORS_H\n#define CM_RESTART_NATIVE_VECTORS_H\n#include <stdint.h>\nstatic const uint32_t cm_rn_hashes[320]={\n'
for r in rows:
 h=2166136261
 for w in r[2:]:
  for b in [w&255,w>>8]:h=((h^b)*16777619)&0xffffffff
 s+=' UINT32_C(0x%08x),\n'%h
s+='};\nstatic const uint16_t cm_rn_indices[8]={'+','.join(map(str,indices))+'};\nstatic const uint16_t cm_rn_full[8][434]={\n'+''.join(' {'+','.join(map(str,rows[i][2:]))+'},\n' for i in indices)+'};\n#endif\n'
(R/'tests/cm_restart_native_vectors.h').write_text(s)
print('Frozen320 CM complete-state fingerprints and8full samples')
