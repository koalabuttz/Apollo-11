#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import gzip, json, pathlib, struct, hashlib
R = pathlib.Path(__file__).resolve().parents[1]
B = R/'build'
proof = json.loads((B/'cm-interbank-extended-proof.json').read_text())
inputs = [kind*65536+a for kind in range(14) for a in [0, 0o40000, 0o100000, 0o177777]]
selected = set(inputs)
rows = {}
h, count = hashlib.sha256(), 0
with gzip.open(B/'cm-interbank-extended-reference.snapshots.bin.gz', 'rb') as source, gzip.open(B/'cm-interbank-extended-native.snapshots.bin.gz', 'rb') as native:
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
assert h.hexdigest() == proof['snapshot_sha256'] and count == 917504 and len(rows) == 56
def fnv(words):
    h = 2166136261
    for x in words:
        h = ((h^(x&255))*16777619)&0xffffffff
        h = ((h^(x>>8))*16777619)&0xffffffff
    return h
s = '/* SPDX-License-Identifier: MIT; source corpus '+proof['snapshot_sha256']+' */\n#ifndef CM_INTERBANK_EXTENDED_VECTORS_H\n#define CM_INTERBANK_EXTENDED_VECTORS_H\n#include <stdint.h>\n#define CM_IB_EXT_TARGET_CASES 56u\n'
s += 'static const uint32_t cm_ib_ext_indices[56]={'+','.join(str(n)+'UL' for n in inputs)+'};\n#ifndef CM_IB_EXT_INDEX_ONLY\n'
s += 'static const uint32_t cm_ib_ext_hash[56]={'+','.join(str(fnv(rows[n]))+'UL' for n in inputs)+'};\n'
s += 'static const uint16_t cm_ib_ext_samples[2][2054]={'+','.join('{'+','.join(map(str, rows[inputs[i]]))+'}' for i in [0,24])+'};\n#endif\n#endif\n'
(R/'tests/cm_interbank_extended_vectors.h').write_text(s)
(B/'cm-interbank-extended-final-source-sample.bin').write_bytes(struct.pack('<2048H', *rows[inputs[-1]][4:2052]))
print('PASS: source-derived56 target contexts, fourteen groups and both TS overflow classes, one explicit invalid BANKJUMP guard')
