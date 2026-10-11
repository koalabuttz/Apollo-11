#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import gzip, json, pathlib, struct, hashlib
R = pathlib.Path(__file__).resolve().parents[1]
B = R/'build'
proof = json.loads((B/'cm-priority-complete-proof.json').read_text())
inputs = [base+14*c for c in [0,1,63,64,255,256,470,1023] for base in range(14)]
selected = set(inputs)
rows = {}
h, count = hashlib.sha256(), 0
with gzip.open(B/'cm-priority-complete-reference.snapshots.bin.gz', 'rb') as source, gzip.open(B/'cm-priority-complete-native.snapshots.bin.gz', 'rb') as native:
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
assert h.hexdigest() == proof['snapshot_sha256'] and count == 14336 and len(rows) == 112
def fnv(words):
    h = 2166136261
    for x in words:
        h = ((h^(x&255))*16777619)&0xffffffff
        h = ((h^(x>>8))*16777619)&0xffffffff
    return h
s = '/* SPDX-License-Identifier: MIT; source corpus '+proof['snapshot_sha256']+' */\n#ifndef CM_PRIORITY_COMPLETE_VECTORS_H\n#define CM_PRIORITY_COMPLETE_VECTORS_H\n#include <stdint.h>\n#define CM_PRIO_TARGET_CASES 112u\n'
s += 'static const uint32_t cm_prio_indices[112]={'+','.join(str(n)+'UL' for n in inputs)+'};\n#ifndef CM_PRIO_INDEX_ONLY\n'
s += 'static const uint32_t cm_prio_hash[112]={'+','.join(str(fnv(rows[n]))+'UL' for n in inputs)+'};\n'
s += 'static const uint16_t cm_prio_samples[2][2054]={'+','.join('{'+','.join(map(str, rows[inputs[i]]))+'}' for i in [0,111])+'};\n#endif\n#endif\n'
(R/'tests/cm_priority_complete_vectors.h').write_text(s)
(B/'cm-priority-complete-final-source-sample.bin').write_bytes(struct.pack('<2048H', *rows[inputs[-1]][4:2052]))
print('PASS: source-derived112 priority contexts, current or allsix selectedcores and both selectedtypes')
