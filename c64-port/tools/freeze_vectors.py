#!/usr/bin/env python3
"""Freeze original-assembly state fingerprints after a successful differential run.
FNV-1a is a compact C64 regression check, not a proof or cryptographic hash.
The host differential comparison remains a complete word-by-word comparison.
"""
import hashlib
import pathlib
ROOT=pathlib.Path(__file__).resolve().parents[1]
p=ROOT/'build/reference.snapshots'
assert p.read_bytes()==(ROOT/'build/native.snapshots').read_bytes()
rows=p.read_text().splitlines()
assert len(rows)==26
digest=[]
for n,row in enumerate(rows):
    values=list(map(int,row.split()))
    assert values[0]==n and len(values)==326
    h=2166136261
    for v in values[1:]:
        w=v & 65535
        for b in [w & 255,w >> 8]:
            h=((h ^ b)*16777619)&0xffffffff
    digest.append(h)
header='/* SPDX-License-Identifier: MIT\n * GENERATED from original assembly snapshots by tools/freeze_vectors.py.\n * Snapshot SHA256: '+hashlib.sha256(p.read_bytes()).hexdigest()+'\n * See docs/VERIFICATION.md for exact reference fixture and boundaries. */\n'
header+='#ifndef REFERENCE_VECTORS_H\n#define REFERENCE_VECTORS_H\n#include <stdint.h>\nstatic const uint32_t reference_digest[26]={\n'
header+=''.join('    UINT32_C(0x%08x), /* scenario %u */\n'%(h,n) for n,h in enumerate(digest))
header+='};\n'
dp=ROOT/'build/dispatch-reference.snapshots'
assert dp.read_bytes()==(ROOT/'build/dispatch-native.snapshots').read_bytes()
drows=dp.read_text().splitlines()
assert len(drows)==109
header+='/* Dispatch snapshot SHA256: '+hashlib.sha256(dp.read_bytes()).hexdigest()+' */\n'
header+='static const uint32_t dispatch_reference_digest[109]={\n'
for n,row in enumerate(drows):
    values=list(map(int,row.split())); assert len(values)==329
    h=2166136261
    for v in values[2:]:
        w=v & 65535
        for b in [w & 255,w >> 8]: h=((h ^ b)*16777619)&0xffffffff
    header+='    UINT32_C(0x%08x), /* case %u step %u */\n'%(h,values[0],values[1])
header+='};\n#endif\n'
(ROOT/'tests/reference_vectors.h').write_text(header)
print('Frozen 26 admission/completion and 109 dispatch assembly-derived fingerprints; '+hashlib.sha256(p.read_bytes()).hexdigest())
