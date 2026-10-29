#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import gzip, json, pathlib, struct, hashlib
R = pathlib.Path(__file__).resolve().parents[1]
B = R/'build'
proof = json.loads((B/'cm-alarm2-image-proof.json').read_text())
inputs=[0,1,2,3,691,692,4721,15050,31170,32767]
seen=set()
for n in range(32768):
 caller=n%4030;qword=64+caller
 if qword<1024:continue
 if qword>=3072:bank=3
 elif qword>=2048:bank=2
 else:
  bank=caller%32
  if bank>=24 and ((caller//32%8)*16)&64:bank+=8
 group=(bank*1024+(qword&1023))//7168
 if group not in seen:
  if n not in inputs:inputs.append(n)
  seen.add(group)
assert seen==set(range(6)),seen
for n in list(range(32768,32800))+[36864,36927,37055,37567,38143,38911,39679,40447,65535]+list(range(32720,32768)):
 if n not in inputs:inputs.append(n)
 if len(inputs)==64:break
assert len(inputs)==64

selected = set(inputs)
rows = {}
h, count = hashlib.sha256(), 0
with gzip.open(B/'cm-alarm2-image-reference.snapshots.bin.gz', 'rb') as source, gzip.open(B/'cm-alarm2-image-native.snapshots.bin.gz', 'rb') as native:
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
assert h.hexdigest() == proof['snapshot_sha256'] and count == 65536 and len(rows) == 64
def fnv(words):
    h = 2166136261
    for x in words:
        h = ((h^(x&255))*16777619)&0xffffffff
        h = ((h^(x>>8))*16777619)&0xffffffff
    return h
s = '/* SPDX-License-Identifier: MIT; source corpus '+proof['snapshot_sha256']+' */\n#ifndef CM_ALARM2_IMAGE_VECTORS_H\n#define CM_ALARM2_IMAGE_VECTORS_H\n#include <stdint.h>\n#define CM_ALARM2_IMAGE_TARGET_CASES 64u\n'
s += 'static const uint32_t cm_alarm2_image_indices[64]={'+','.join(str(n)+'UL' for n in inputs)+'};\n#ifndef CM_ALARM2_IMAGE_INDEX_ONLY\n'
s += 'static const uint32_t cm_alarm2_image_hash[64]={'+','.join(str(fnv(rows[n]))+'UL' for n in inputs)+'};\n'
s += 'static const uint16_t cm_alarm2_image_samples[2][2065]={'+','.join('{'+','.join(map(str, rows[inputs[i]]))+'}' for i in [0,63])+'};\n#endif\n#endif\n'
(R/'tests/cm_alarm2_image_vectors.h').write_text(s)
(B/'cm-alarm2-image-final-source-sample.bin').write_bytes(struct.pack('<2048H', *rows[inputs[-1]][4:2052]))
print('PASS: source-derived64 ALARM caller-data contexts, post-store aliases, allEB and passive original data pages')
