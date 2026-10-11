#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import gzip, json, pathlib, struct, hashlib
R = pathlib.Path(__file__).resolve().parents[1]
B = R/'build'
proof = json.loads((B/'cm-abort-word-proof.json').read_text())
inputs=[0,1,2,2073,2076,567,570,573,27021,27024,16384,32767,32768,65535,65536]
for eb in range(8):
 for entry in range(3):
  n=eb*12090+entry
  if n not in inputs:inputs.append(n)
seen=set()
for caller in range(960,4030):
 q=64+caller
 if q>=3072:bank=3
 elif q>=2048:bank=2
 else:
  bank=caller%32
  if bank>=24 and ((caller//32%8)*16)&64:bank+=8
 group=(bank*1024+(q&1023))//7168
 if group not in seen:
  n=caller*3
  if n not in inputs:inputs.append(n)
  seen.add(group)
assert seen==set(range(6)),seen
for n in range(96690,96720):
 if n not in inputs:inputs.append(n)
 if len(inputs)==64:break
assert len(inputs)==64,len(inputs)

selected = set(inputs)
rows = {}
h, count = hashlib.sha256(), 0
with gzip.open(B/'cm-abort-word-reference.snapshots.bin.gz', 'rb') as source, gzip.open(B/'cm-abort-word-native.snapshots.bin.gz', 'rb') as native:
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
assert h.hexdigest() == proof['snapshot_sha256'] and count == 96720 and len(rows) == 64
def fnv(words):
    h = 2166136261
    for x in words:
        h = ((h^(x&255))*16777619)&0xffffffff
        h = ((h^(x>>8))*16777619)&0xffffffff
    return h
s = '/* SPDX-License-Identifier: MIT; source corpus '+proof['snapshot_sha256']+' */\n#ifndef CM_ABORT_WORD_VECTORS_H\n#define CM_ABORT_WORD_VECTORS_H\n#include <stdint.h>\n#define CM_ABORT_WORD_TARGET_CASES 64u\n'
s += 'static const uint32_t cm_abort_word_indices[64]={'+','.join(str(n)+'UL' for n in inputs)+'};\n#ifndef CM_ABORT_WORD_INDEX_ONLY\n'
s += 'static const uint32_t cm_abort_word_hash[64]={'+','.join(str(fnv(rows[n]))+'UL' for n in inputs)+'};\n'
s += 'static const uint16_t cm_abort_word_samples[2][2065]={'+','.join('{'+','.join(map(str, rows[inputs[i]]))+'}' for i in [0,63])+'};\n#endif\n#endif\n'
(R/'tests/cm_abort_word_vectors.h').write_text(s)
(B/'cm-abort-word-final-source-sample.bin').write_bytes(struct.pack('<2048H', *rows[inputs[-1]][4:2052]))
print('PASS: source-derived64 post-snapshot abort caller-word contexts, post-store aliases, allEB and passive original data pages')
