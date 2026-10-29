#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import gzip, json, pathlib, struct, hashlib
R = pathlib.Path(__file__).resolve().parents[1]
B = R/'build'
proof = json.loads((B/'cm-alarm-complete-proof.json').read_text())
inputs=[]
for eb in range(8):
 for irq in range(2):
  for entry in range(4):
   caller=eb*512+5
   if eb==5 and entry==3:caller=2960
   inputs.append(caller*8+irq*4+entry)
assert len(inputs)==64 and len(set(inputs))==64

selected = set(inputs)
rows = {}
h, count = hashlib.sha256(), 0
with gzip.open(B/'cm-alarm-complete-reference.snapshots.bin.gz', 'rb') as source, gzip.open(B/'cm-alarm-complete-native.snapshots.bin.gz', 'rb') as native:
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
assert h.hexdigest() == proof['snapshot_sha256'] and count == 32240 and len(rows) == 64
def fnv(words):
    h = 2166136261
    for x in words:
        h = ((h^(x&255))*16777619)&0xffffffff
        h = ((h^(x>>8))*16777619)&0xffffffff
    return h
s = '/* SPDX-License-Identifier: MIT; source corpus '+proof['snapshot_sha256']+' */\n#ifndef CM_ALARM_COMPLETE_VECTORS_H\n#define CM_ALARM_COMPLETE_VECTORS_H\n#include <stdint.h>\n#define CM_ALARM_COMPLETE_TARGET_CASES 64u\n'
s += 'static const uint32_t cm_alarm_complete_indices[64]={'+','.join(str(n)+'UL' for n in inputs)+'};\n#ifndef CM_ALARM_COMPLETE_INDEX_ONLY\n'
s += 'static const uint32_t cm_alarm_complete_hash[64]={'+','.join(str(fnv(rows[n]))+'UL' for n in inputs)+'};\n'
s += 'static const uint16_t cm_alarm_complete_samples[2][2065]={'+','.join('{'+','.join(map(str, rows[inputs[i]]))+'}' for i in [0,63])+'};\n#endif\n#endif\n'
(R/'tests/cm_alarm_complete_vectors.h').write_text(s)
(B/'cm-alarm-complete-final-source-sample.bin').write_bytes(struct.pack('<2048H', *rows[inputs[-1]][4:2052]))
print('PASS: source-derived64 ALARM caller-data contexts, post-store aliases, allEB and passive original data pages')
