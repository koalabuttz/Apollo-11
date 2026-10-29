#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze complete rotation source states losslessly."""
import hashlib,pathlib
from frozen_words import c_header,encode_rows
root=pathlib.Path(__file__).resolve().parents[1];data=(root/'build/rotation-reference.snapshots').read_bytes()
assert data==(root/'build/rotation-native.snapshots').read_bytes()
rows=[list(map(int,r.split()))for r in data.decode().splitlines()]
assert len(rows)==32768 and all(len(r)==133 for r in rows)
indices=list(range(64))+list(range(4096,4112))+list(range(8192,8208))+list(range(12288,12320))
assert len(indices)==128
packed=encode_rows([rows[i][1:]for i in indices])
s='/* SPDX-License-Identifier: MIT\n * Generated lossless original assembly/hardware-DV rotation states.\n * Snapshot SHA256:'+hashlib.sha256(data).hexdigest()+' */\n#ifndef ROTATION_VECTORS_H\n#define ROTATION_VECTORS_H\n'+c_header(packed,'rotation')
s+='static const uint16_t rotation_indices[ROTATION_FROZEN_ROWS]={'+','.join(map(str,indices))+'};\n#endif\n'
(root/'tests/rotation_vectors.h').write_text(s)
print('128 complete 132-word states;',packed.storage_bytes,'packed bytes +256 index bytes')
