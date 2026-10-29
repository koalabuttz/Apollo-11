#!/usr/bin/env python3
"""Freeze lifecycle fingerprints only after exact independent assembly agreement."""
import hashlib
import pathlib
root=pathlib.Path(__file__).resolve().parents[1]
original=root/'build/lifecycle-reference.snapshots'
assert original.read_bytes()==(root/'build/lifecycle-native.snapshots').read_bytes()
rows=[list(map(int,r.split())) for r in original.read_text().splitlines()]
assert len(rows)==115 and all(len(r)==329 for r in rows)
header=('/* SPDX-License-Identifier: MIT\n * GENERATED from ORIGINAL assembly by tools/freeze_lifecycle.py.\n'
        '* Snapshot SHA256: '+hashlib.sha256(original.read_bytes()).hexdigest()+'\n */\n'
        '#ifndef LIFECYCLE_VECTORS_H\n#define LIFECYCLE_VECTORS_H\n#include <stdint.h>\n'
        'static const uint32_t lifecycle_reference_digest[115]={\n')
for row in rows:
    h=2166136261
    for v in row[2:]:
        w=v & 65535
        for b in (w & 255,w >> 8): h=((h ^ b)*16777619)&0xffffffff
    header+='    UINT32_C(0x%08x), /* case %u step %u */\n'%(h,row[0],row[1])
header+='};\n#endif\n'
(root/'tests/lifecycle_vectors.h').write_text(header)
print('Frozen 115 assembly-derived lifecycle fingerprints')
