#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Regenerate representative abort snapshot vectors solely from identical original/native streams."""
import gzip, hashlib, json, pathlib, struct
R = pathlib.Path(__file__).resolve().parents[1]
B = R/'build'
proof = json.loads((B/'cm-abort-snapshot-proof.json').read_text())
indices = [3*c+b for c in [0,1,63,511,1023,4095] for b in range(3)]
rows = {n: [] for n in indices}
h, count = hashlib.sha256(), 0
with gzip.open(B/'cm-abort-snapshot-reference.snapshots.bin.gz','rb') as src, gzip.open(B/'cm-abort-snapshot-native.snapshots.bin.gz','rb') as native:
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
assert count == 61440 and h.hexdigest() == proof['snapshot_sha256']
assert sum(len(rows[n]) for n in indices) == 90

def fnv(words):
    h = 2166136261
    for x in words:
        h = ((h^(x&255))*16777619)&0xffffffff
        h = ((h^(x>>8))*16777619)&0xffffffff
    return h
s = '/* SPDX-License-Identifier: MIT; complete original abort snapshot source states */\n#ifndef CM_ABORT_VECTORS_H\n#define CM_ABORT_VECTORS_H\n#include <stdint.h>\n#define CM_ABORT_TARGET_CASES 18u\n'
s += 'static const uint16_t cm_abort_indices[18]={'+','.join(map(str,indices))+'};\n'
s += 'static const uint8_t cm_abort_counts[18]={'+','.join(str(len(rows[n])) for n in indices)+'};\n'
s += 'static const uint32_t cm_abort_hash[18][5]={'+','.join('{'+','.join(str(fnv(r[2:]))+'u' for r in rows[n])+'}' for n in indices)+'};\n'
samples = [rows[indices[0]][0][4:], rows[indices[-1]][-1][4:]]
s += 'static const uint16_t cm_abort_sample[2][2050]={'+','.join('{'+','.join(map(str,r))+'}' for r in samples)+'};\n#endif\n'
target = R/'tests/cm_abort_snapshot_vectors.h'
if target.exists(): assert target.read_text() == s, 'Existing source-derived vectors differ'
else: target.write_text(s)
final = struct.pack('<2048H',*samples[-1][:2048])
target = B/'cm-abort-snapshot-final-source-sample.bin'
if target.exists(): assert target.read_bytes() == final
else: target.write_bytes(final)
print('PASS: regenerated18 original abort snapshot contexts/90 complete source boundaries, two complete image/I-O samples')
