#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import gzip, json, pathlib, struct, hashlib
R = pathlib.Path(__file__).resolve().parents[1]
B = R/'build'
proof = json.loads((B/'cm-enema-image-proof.json').read_text())
inputs = [0,1,2,3,6,7,15,31,63,64,127,191,192,193,255,511,512,767,1023,1279,1535,1791,2047,2303,2559,2815,3071,3327,3583,3839,4094,4095]
selected = set(inputs)
rows = {}
h, count = hashlib.sha256(), 0
with gzip.open(B/'cm-enema-image-reference.snapshots.bin.gz', 'rb') as source, gzip.open(B/'cm-enema-image-native.snapshots.bin.gz', 'rb') as native:
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
assert h.hexdigest() == proof['snapshot_sha256'] and count == 4096 and len(rows) == 32
def fnv(words):
    h = 2166136261
    for x in words:
        h = ((h^(x&255))*16777619)&0xffffffff
        h = ((h^(x>>8))*16777619)&0xffffffff
    return h
s = '/* SPDX-License-Identifier: MIT; source corpus '+proof['snapshot_sha256']+' */\n#ifndef CM_ENEMA_IMAGE_VECTORS_H\n#define CM_ENEMA_IMAGE_VECTORS_H\n#include <stdint.h>\n#define CM_ENEMA_IMAGE_TARGET_CASES 32u\n'
s += 'static const uint32_t cm_enema_image_indices[32]={'+','.join(str(n)+'UL' for n in inputs)+'};\n#ifndef CM_ENEMA_IMAGE_INDEX_ONLY\n'
s += 'static const uint32_t cm_enema_image_hash[32]={'+','.join(str(fnv(rows[n]))+'UL' for n in inputs)+'};\n'
s += 'static const uint16_t cm_enema_image_samples[2][2065]={'+','.join('{'+','.join(map(str, rows[inputs[i]]))+'}' for i in [0,31])+'};\n#endif\n#endif\n'
(R/'tests/cm_enema_image_vectors.h').write_text(s)
(B/'cm-enema-image-final-source-sample.bin').write_bytes(struct.pack('<2048H', *rows[inputs[-1]][4:2052]))
print('PASS: source-derived32 whole ENEMA contexts, fresh/display/bad-phase and TVC continuation domains')
