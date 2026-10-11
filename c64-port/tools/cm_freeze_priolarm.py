#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import gzip, json, pathlib, struct, hashlib
R = pathlib.Path(__file__).resolve().parents[1]
B = R/'build'
proof = json.loads((B/'cm-priolarm-proof.json').read_text())
inputs = [0,1,2,3,6,7,15,31,127,255,511,1023,2047,4095,8191,16383,16384,16385,24575,32767,32768,32769,40959,49151,49152,49153,57343,61439,65023,65279,65534,65535]
selected = set(inputs)
rows = {}
h, count = hashlib.sha256(), 0
with gzip.open(B/'cm-priolarm-reference.snapshots.bin.gz', 'rb') as source, gzip.open(B/'cm-priolarm-native.snapshots.bin.gz', 'rb') as native:
    while True:
        a, b = source.read(4130), native.read(4130)
        assert a == b
        if not a:
            break
        assert len(a) == 4130
        h.update(a)
        count += 1
        r = struct.unpack('<2065H', a)
        n = r[0] | (r[1]<<16)
        if n in selected:
            rows[n] = r
assert h.hexdigest() == proof['snapshot_sha256'] and count == 65536 and len(rows) == 32
def fnv(words):
    h = 2166136261
    for x in words:
        h = ((h^(x&255))*16777619)&0xffffffff
        h = ((h^(x>>8))*16777619)&0xffffffff
    return h
s = '/* SPDX-License-Identifier: MIT; source corpus '+proof['snapshot_sha256']+' */\n#ifndef CM_PRIOLARM_VECTORS_H\n#define CM_PRIOLARM_VECTORS_H\n#include <stdint.h>\n#define CM_PRIOLARM_TARGET_CASES 32u\n'
s += 'static const uint32_t cm_priolarm_indices[32]={'+','.join(str(n)+'UL' for n in inputs)+'};\n#ifndef CM_PRIOLARM_INDEX_ONLY\n'
s += 'static const uint32_t cm_priolarm_hash[32]={'+','.join(str(fnv(rows[n]))+'UL' for n in inputs)+'};\n'
s += 'static const uint16_t cm_priolarm_samples[2][2065]={'+','.join('{'+','.join(map(str, rows[inputs[i]]))+'}' for i in [0,31])+'};\n#endif\n#endif\n'
(R/'tests/cm_priolarm_vectors.h').write_text(s)
(B/'cm-priolarm-final-source-sample.bin').write_bytes(struct.pack('<2048H', *rows[inputs[-1]][4:2052]))
print('PASS: source-derived32 PRIOLARM contexts, raw register overflow and alarm-record corner domains')
