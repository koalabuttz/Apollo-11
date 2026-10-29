#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import gzip, json, pathlib, struct, hashlib
R = pathlib.Path(__file__).resolve().parents[1]
B = R/'build'
proof = json.loads((B/'cm-phase-cleanup-proof.json').read_text())
inputs = [3*c+b for c in [0,1,2,63,64,191,192,193,255,511,512,1023,1535,2047,3071,4095] for b in range(3)]
selected = set(inputs)
rows = {}
h, count = hashlib.sha256(), 0
with gzip.open(B/'cm-phase-cleanup-reference.snapshots.bin.gz', 'rb') as source, gzip.open(B/'cm-phase-cleanup-native.snapshots.bin.gz', 'rb') as native:
    while True:
        a, b = source.read(4108), native.read(4108)
        assert a == b
        if not a:
            break
        assert len(a) == 4108
        h.update(a)
        count += 1
        r = struct.unpack('<2054H', a)
        n = r[0] | (r[1]<<16)
        if n in selected:
            rows[n] = r
assert h.hexdigest() == proof['snapshot_sha256'] and count == 12288 and len(rows) == 48
def fnv(words):
    h = 2166136261
    for x in words:
        h = ((h^(x&255))*16777619)&0xffffffff
        h = ((h^(x>>8))*16777619)&0xffffffff
    return h
s = '/* SPDX-License-Identifier: MIT; source corpus '+proof['snapshot_sha256']+' */\n#ifndef CM_PHASE_CLEANUP_VECTORS_H\n#define CM_PHASE_CLEANUP_VECTORS_H\n#include <stdint.h>\n#define CM_CLEANUP_TARGET_CASES 48u\n'
s += 'static const uint32_t cm_cleanup_indices[48]={'+','.join(str(n)+'UL' for n in inputs)+'};\n#ifndef CM_CLEANUP_INDEX_ONLY\n'
s += 'static const uint32_t cm_cleanup_hash[48]={'+','.join(str(fnv(rows[n]))+'UL' for n in inputs)+'};\n'
s += 'static const uint16_t cm_cleanup_samples[2][2054]={'+','.join('{'+','.join(map(str, rows[inputs[i]]))+'}' for i in [0,47])+'};\n#endif\n#endif\n'
(R/'tests/cm_phase_cleanup_vectors.h').write_text(s)
(B/'cm-phase-cleanup-final-source-sample.bin').write_bytes(struct.pack('<2048H', *rows[inputs[-1]][4:2052]))
print('PASS: source-derived48 whole phase cleanup contexts, all three genuine entries')
