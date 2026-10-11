#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import gzip, json, pathlib, struct, hashlib
R = pathlib.Path(__file__).resolve().parents[1]
B = R/'build'
proof = json.loads((B/'cm-alarm-record-proof.json').read_text())
inputs = [4*c+b for c in [0,1,2,4,7,8,9,16,64,65,72,73,511,1023,2047,4095] for b in range(4)]
selected = set(inputs)
rows = {}
h, count = hashlib.sha256(), 0
with gzip.open(B/'cm-alarm-record-reference.snapshots.bin.gz', 'rb') as source, gzip.open(B/'cm-alarm-record-native.snapshots.bin.gz', 'rb') as native:
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
assert h.hexdigest() == proof['snapshot_sha256'] and count == 16384 and len(rows) == 64
def fnv(words):
    h = 2166136261
    for x in words:
        h = ((h^(x&255))*16777619)&0xffffffff
        h = ((h^(x>>8))*16777619)&0xffffffff
    return h
s = '/* SPDX-License-Identifier: MIT; source corpus '+proof['snapshot_sha256']+' */\n#ifndef CM_ALARM_RECORD_VECTORS_H\n#define CM_ALARM_RECORD_VECTORS_H\n#include <stdint.h>\n#define CM_ALARM_TARGET_CASES 64u\n'
s += 'static const uint32_t cm_alarm_indices[64]={'+','.join(str(n)+'UL' for n in inputs)+'};\n#ifndef CM_ALARM_INDEX_ONLY\n'
s += 'static const uint32_t cm_alarm_hash[64]={'+','.join(str(fnv(rows[n]))+'UL' for n in inputs)+'};\n'
s += 'static const uint16_t cm_alarm_samples[2][2054]={'+','.join('{'+','.join(map(str, rows[inputs[i]]))+'}' for i in [0,63])+'};\n#endif\n#endif\n'
(R/'tests/cm_alarm_record_vectors.h').write_text(s)
(B/'cm-alarm-record-final-source-sample.bin').write_bytes(struct.pack('<2048H', *rows[inputs[-1]][4:2052]))
print('PASS: source-derived64 alarm contexts, ordinary returns and source-selected POODOO frontier')
