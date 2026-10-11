#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze complete throttle_feedback source states losslessly."""
import hashlib,pathlib
from frozen_words import c_header,encode_rows
root=pathlib.Path(__file__).resolve().parents[1];data=(root/'build/throttle_feedback-reference.snapshots').read_bytes()
assert data==(root/'build/throttle_feedback-native.snapshots').read_bytes()
rows=[list(map(int,r.split()))for r in data.decode().splitlines()]
assert len(rows)==16384 and all(len(r)==126 for r in rows)
indices=list(range(24))+[64,127,128,255,256,511,512,1023,1024,2047,2048,4095]+list(range(4096,4110))+list(range(8192,8206))
assert len(indices)==64
packed=encode_rows([rows[i][1:]for i in indices])
s='/* SPDX-License-Identifier: MIT\n * Generated lossless original assembly/hardware-DV throttle_feedback states.\n * Snapshot SHA256:'+hashlib.sha256(data).hexdigest()+' */\n#ifndef THROTTLE_FEEDBACK_VECTORS_H\n#define THROTTLE_FEEDBACK_VECTORS_H\n'+c_header(packed,'throttle_feedback')
s+='static const uint16_t throttle_feedback_indices[THROTTLE_FEEDBACK_FROZEN_ROWS]={'+','.join(map(str,indices))+'};\n#endif\n'
(root/'tests/throttle_feedback_vectors.h').write_text(s)
print('64 complete 125-word states;',packed.storage_bytes,'packed bytes +128 index bytes')

from collections import Counter
print("Selected clamps:",Counter(rows[i][1]for i in indices))
