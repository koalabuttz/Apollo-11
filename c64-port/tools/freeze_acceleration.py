#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze complete acceleration source states losslessly."""
import hashlib,pathlib
from frozen_words import c_header,encode_rows
root=pathlib.Path(__file__).resolve().parents[1];data=(root/'build/acceleration-reference.snapshots').read_bytes()
assert data==(root/'build/acceleration-native.snapshots').read_bytes()
rows=[list(map(int,r.split()))for r in data.decode().splitlines()]
assert len(rows)==16384 and all(len(r)==114 for r in rows)
indices=list(range(48))+[64,127,128,255,256,511,512,1023,1024,2047,2048,4095]+list(range(4096,4114))+list(range(8192,8210))
assert len(indices)==96
packed=encode_rows([rows[i][1:]for i in indices])
s='/* SPDX-License-Identifier: MIT\n * Generated lossless original assembly/hardware-DV acceleration states.\n * Snapshot SHA256:'+hashlib.sha256(data).hexdigest()+' */\n#ifndef ACCELERATION_VECTORS_H\n#define ACCELERATION_VECTORS_H\n'+c_header(packed,'acceleration')
s+='static const uint16_t acceleration_indices[ACCELERATION_FROZEN_ROWS]={'+','.join(map(str,indices))+'};\n#endif\n'
(root/'tests/acceleration_vectors.h').write_text(s)
print('96 complete 113-word states;',packed.storage_bytes,'packed bytes +192 index bytes')
