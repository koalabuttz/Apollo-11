#!/usr/bin/env python3
"""Freeze fingerprints only from independently matched original assembly snapshots."""
import pathlib, hashlib
root=pathlib.Path(__file__).resolve().parents[1]
a=(root/'build/restart-reference.snapshots').read_bytes()
assert a==(root/'build/restart-native.snapshots').read_bytes()
rows=[list(map(int,r.split())) for r in a.decode().splitlines()]
assert len(rows)==226 and all(len(r)==451 for r in rows)
s=('/* SPDX-License-Identifier: MIT\n * GENERATED from original assembly by tools/freeze_restart.py.\n * SHA256: '+hashlib.sha256(a).hexdigest()+'\n */\n#ifndef RESTART_VECTORS_H\n#define RESTART_VECTORS_H\n#include <stdint.h>\nstatic const uint32_t restart_reference_digest[226]={\n')
for r in rows:
    h=2166136261
    for v in r[2:]:
        for b in (v&255,(v>>8)&255): h=((h ^ b)*16777619)&0xffffffff
    s+='    UINT32_C(0x%08x), /* %u/%u */\n'%(h,*r[:2])
s+='};\n#endif\n'; (root/'tests/restart_vectors.h').write_text(s)
print('Frozen 226 original-assembly fingerprints')
