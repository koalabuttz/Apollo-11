#!/usr/bin/env python3
"""Freeze complete selected shift states after independent equality."""
import hashlib,pathlib
from frozen_words import encode_rows,c_header
root=pathlib.Path(__file__).resolve().parents[1]
data=(root/'build/shift-reference.snapshots').read_bytes()
assert data==(root/'build/shift-native.snapshots').read_bytes()
rows=[list(map(int,line.split()))for line in data.decode().splitlines()]
assert len(rows)==26880 and all(len(row)==90 for row in rows)
selected=[rows[group*320+local] for group in range(84)for local in [37,137]]
s=('/* SPDX-License-Identifier: MIT\n * GENERATED from unchanged assembly by tools/freeze_shift.py.\n'
   ' * Full snapshot SHA256: '+hashlib.sha256(data).hexdigest()+'\n */\n'
   '#ifndef SHIFT_VECTORS_H\n#define SHIFT_VECTORS_H\n')
packed=encode_rows([row[1:]for row in selected]);s+=c_header(packed,'shift')
s+='#endif\n';(root/'tests/shift_vectors.h').write_text(s)
print('Frozen 168 complete 89-word states covering every general and short shift count/form')
print('Lossless expected-state storage bytes:',packed.storage_bytes)
