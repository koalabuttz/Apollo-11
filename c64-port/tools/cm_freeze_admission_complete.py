#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import gzip, json, pathlib, struct, hashlib
R = pathlib.Path(__file__).resolve().parents[1]
B = R/'build'
proof = json.loads((B/'cm-admission-complete-proof.json').read_text())
base = [entry+core*4+vac*32 for vac in [0,1,4,5] for core in [0,1,3,6,7] for entry in range(4)]
inputs = [n+192*(i%64) for i,n in enumerate(base)] + [n+192*(48+i) for i,n in enumerate(range(8,23))] + [12287]
selected = set(inputs)
rows = {}
h, count = hashlib.sha256(), 0
with gzip.open(B/'cm-admission-complete-reference.snapshots.bin.gz', 'rb') as source, gzip.open(B/'cm-admission-complete-native.snapshots.bin.gz', 'rb') as native:
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
assert h.hexdigest() == proof['snapshot_sha256'] and count == 12288 and len(rows) == 96
def fnv(words):
    h = 2166136261
    for x in words:
        h = ((h^(x&255))*16777619)&0xffffffff
        h = ((h^(x>>8))*16777619)&0xffffffff
    return h
s = '/* SPDX-License-Identifier: MIT; source corpus '+proof['snapshot_sha256']+' */\n#ifndef CM_ADMISSION_COMPLETE_VECTORS_H\n#define CM_ADMISSION_COMPLETE_VECTORS_H\n#include <stdint.h>\n#define CM_ADMIT_TARGET_CASES 96u\n'
s += 'static const uint32_t cm_admit_indices[96]={'+','.join(str(n)+'UL' for n in inputs)+'};\n#ifndef CM_ADMIT_INDEX_ONLY\n'
s += 'static const uint32_t cm_admit_hash[96]={'+','.join(str(fnv(rows[n]))+'UL' for n in inputs)+'};\n'
s += 'static const uint16_t cm_admit_samples[2][2054]={'+','.join('{'+','.join(map(str, rows[inputs[i]]))+'}' for i in [0,95])+'};\n#endif\n#endif\n'
(R/'tests/cm_admission_complete_vectors.h').write_text(s)
(B/'cm-admission-complete-final-source-sample.bin').write_bytes(struct.pack('<2048H', *rows[inputs[-1]][4:2052]))
print('PASS: source-derived96 target contexts, all four entries and representative resource corners with bank variation')
