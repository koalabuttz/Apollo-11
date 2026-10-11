#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Regenerate representative uninterrupted abort lifecycle vectors solely from identical original/native streams."""
import gzip, hashlib, json, pathlib, struct
R = pathlib.Path(__file__).resolve().parents[1]
B = R/'build'
proof = json.loads((B/'cm-abort-lifecycle-proof.json').read_text())
indices = [3*c+b for c in [0,1,510,511] for b in range(3)]
rows = {n: [] for n in indices}
h, count = hashlib.sha256(), 0
with gzip.open(B/'cm-abort-lifecycle-reference.snapshots.bin.gz','rb') as src, gzip.open(B/'cm-abort-lifecycle-native.snapshots.bin.gz','rb') as native:
    while True:
        a, b = src.read(4132), native.read(4132)
        assert a == b
        if not a: break
        assert len(a) == 4132
        h.update(a); count += 1
        r = struct.unpack('<2066H', a)
        n = r[0] | (r[1]<<16)
        if n in rows:
            assert r[2] == len(rows[n])
            rows[n].append(r)
assert count == 14848 and h.hexdigest() == proof['snapshot_sha256']
assert sum(len(rows[n]) for n in indices) == 116

def fnv(words):
    h = 2166136261
    for x in words:
        h = ((h^(x&255))*16777619)&0xffffffff
        h = ((h^(x>>8))*16777619)&0xffffffff
    return h
s = '/* SPDX-License-Identifier: MIT; complete original uninterrupted abort lifecycle source states */\n#ifndef CM_LIFECYCLE_VECTORS_H\n#define CM_LIFECYCLE_VECTORS_H\n#include <stdint.h>\n#define CM_LIFECYCLE_TARGET_CASES 12u\n'
s += 'static const uint16_t cm_lifecycle_indices[12]={'+','.join(map(str,indices))+'};\n'
s += 'static const uint8_t cm_lifecycle_counts[12]={'+','.join(str(len(rows[n])) for n in indices)+'};\n'
s += 'static const uint32_t cm_lifecycle_hash[12][14]={'+','.join('{'+','.join(str(fnv(r[2:]))+'u' for r in rows[n])+'}' for n in indices)+'};\n'
samples = [rows[indices[0]][0][4:], rows[indices[-1]][-1][4:]]
s += 'static const uint16_t cm_lifecycle_sample[2][2062]={'+','.join('{'+','.join(map(str,r))+'}' for r in samples)+'};\n#endif\n'
target = R/'tests/cm_abort_lifecycle_vectors.h'
if target.exists(): assert target.read_text() == s, 'Existing source-derived vectors differ'
else: target.write_text(s)
final = struct.pack('<2048H',*samples[-1][:2048])
target = B/'cm-abort-lifecycle-final-source-sample.bin'
if target.exists(): assert target.read_bytes() == final
else: target.write_bytes(final)
print('PASS: regenerated12 original uninterrupted abort lifecycle contexts/116 complete source boundaries, two complete image/I-O samples')
