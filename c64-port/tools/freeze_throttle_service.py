#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze complete throttle_service source states losslessly."""
import hashlib,pathlib
from frozen_words import c_header,encode_rows
root=pathlib.Path(__file__).resolve().parents[1];data=(root/'build/throttle_service-reference.snapshots').read_bytes()
assert data==(root/'build/throttle_service-native.snapshots').read_bytes()
rows=[list(map(int,r.split()))for r in data.decode().splitlines()]
assert len(rows)==16384 and all(len(r)==149 for r in rows)
indices=[i*127 for i in range(32)]+list(range(4096,4128))+list(range(8192,8224))
assert len(indices)==96
packed=encode_rows([rows[i][1:]for i in indices])
s='/* SPDX-License-Identifier: MIT\n * Generated lossless original assembly/hardware-DV throttle_service states.\n * Snapshot SHA256:'+hashlib.sha256(data).hexdigest()+' */\n#ifndef THROTTLE_SERVICE_VECTORS_H\n#define THROTTLE_SERVICE_VECTORS_H\n'+c_header(packed,'throttle_service')
s+='static const uint16_t throttle_service_indices[THROTTLE_SERVICE_FROZEN_ROWS]={'+','.join(map(str,indices))+'};\n#endif\n'
(root/'tests/throttle_service_vectors.h').write_text(s)
print('96 complete 148-word states;',packed.storage_bytes,'packed bytes +192 index bytes')

from collections import Counter
print("Selected statuses:",Counter(rows[i][1]for i in indices))
