#!/usr/bin/env python3
"""Freeze source whole-image CM startup and cleanup checkpoints."""
import pathlib,hashlib
R=pathlib.Path(__file__).resolve().parents[1];p=R/'build/cm-startup-reference.snapshots'
assert p.read_bytes()==(R/'build/cm-startup-native.snapshots').read_bytes()
rows=[list(map(int,r.split())) for r in p.read_text().splitlines()]
assert len(rows)==640 and all(len(r)==2064 for r in rows)
indices=[0,128,256,384,512]
s='/* SPDX-License-Identifier: MIT\n * GENERATED from original CM startup assembly. Snapshot SHA256: '+hashlib.sha256(p.read_bytes()).hexdigest()+' */\n#ifndef CM_STARTUP_VECTORS_H\n#define CM_STARTUP_VECTORS_H\n#include <stdint.h>\nstatic const uint32_t cm_startup_hashes[640]={\n'
for r in rows:
 h=2166136261
 for w in r[1:]:
  for b in [w&255,w>>8]:h=((h^b)*16777619)&0xffffffff
 s+=' UINT32_C(0x%08x),\n'%h
s+='};\nstatic const uint16_t cm_startup_indices[5]={'+','.join(map(str,indices))+'};\nstatic const uint16_t cm_startup_full[5][2063]={\n'+''.join(' {'+','.join(map(str,rows[i][1:]))+'},\n' for i in indices)+'};\n#endif\n'
(R/'tests/cm_startup_vectors.h').write_text(s)
print('Frozen640 whole-image source fingerprints and5full2063-value states')
