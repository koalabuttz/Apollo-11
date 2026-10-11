#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Regenerate representative INITDSP vectors solely from identical original/native streams."""
import gzip, hashlib, json, pathlib, struct
R = pathlib.Path(__file__).resolve().parents[1]
B = R/'build'
proof = json.loads((B/'cm-init-display-mixed-proof.json').read_text())
indices = [14*c+b for c in [0,1,63,511] for b in range(14)]
rows = {n: [] for n in indices}
h, count = hashlib.sha256(), 0
with gzip.open(B/'cm-init-display-mixed-reference.snapshots.bin.gz','rb') as src, gzip.open(B/'cm-init-display-mixed-native.snapshots.bin.gz','rb') as native:
    while True:
        a, b = src.read(4108), native.read(4108)
        assert a == b
        if not a: break
        assert len(a) == 4108
        h.update(a); count += 1
        r = struct.unpack('<2054H', a)
        n = r[0] | (r[1]<<16)
        if n in rows:
            assert r[2] == len(rows[n])
            rows[n].append(r)
assert count == 15360 and h.hexdigest() == proof['snapshot_sha256']
assert sum(len(rows[n]) for n in indices) == 120

def fnv(words):
    h = 2166136261
    for x in words:
        h = ((h^(x&255))*16777619)&0xffffffff
        h = ((h^(x>>8))*16777619)&0xffffffff
    return h
s = '/* SPDX-License-Identifier: MIT; complete original INITDSP mixed source states */\n#ifndef CM_INIT_MIXED_VECTORS_H\n#define CM_INIT_MIXED_VECTORS_H\n#include <stdint.h>\n#define CM_INIT_MIXED_TARGET_CASES 56u\n'
s += 'static const uint16_t cm_init_mixed_indices[56]={'+','.join(map(str,indices))+'};\n'
s += 'static const uint8_t cm_init_mixed_counts[56]={'+','.join(str(len(rows[n])) for n in indices)+'};\n'
s += 'static const uint32_t cm_init_mixed_hash[56][3]={'+','.join('{'+','.join(str(fnv(r[2:]))+'u' for r in rows[n])+'}' for n in indices)+'};\n'
samples = [rows[indices[0]][0][4:], rows[indices[-1]][-1][4:]]
s += 'static const uint16_t cm_init_mixed_sample[2][2050]={'+','.join('{'+','.join(map(str,r))+'}' for r in samples)+'};\n#endif\n'
target = R/'tests/cm_init_display_mixed_vectors.h'
if target.exists(): assert target.read_text() == s, 'Existing source-derived vectors differ'
else: target.write_text(s)
final = struct.pack('<2048H',*samples[-1][:2048])
target = B/'cm-init-display-mixed-final-source-sample.bin'
if target.exists(): assert target.read_bytes() == final
else: target.write_bytes(final)
print('PASS: regenerated56 original INITDSP mixed contexts/120 complete source boundaries, two complete image/I-O samples')
