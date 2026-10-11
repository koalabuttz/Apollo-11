#!/usr/bin/env python3
"""Freeze complete selected DOT states after independent equality."""
import hashlib,pathlib
from frozen_words import encode_rows,c_header
root=pathlib.Path(__file__).resolve().parents[1]
data=(root/'build/dot-reference.snapshots').read_bytes()
assert data==(root/'build/dot-native.snapshots').read_bytes()
rows=[list(map(int,line.split()))for line in data.decode().splitlines()]
assert len(rows)==10272 and all(len(row)==101 for row in rows)
indices=[row if row<32 else 32+((row-32)//2)*251+(4096 if row%2 else 0) if row<64 else 8224+(row-64)*61 for row in range(96)]
selected=[rows[index]for index in indices]
s=('/* SPDX-License-Identifier: MIT\n * GENERATED from unchanged assembly by tools/freeze_dot.py.\n'
   ' * Full snapshot SHA256: '+hashlib.sha256(data).hexdigest()+'\n */\n'
   '#ifndef DOT_VECTORS_H\n#define DOT_VECTORS_H\n')
packed=encode_rows([row[1:]for row in selected]);s+=c_header(packed,'dot')
s+='#endif\n';(root/'tests/dot_vectors.h').write_text(s)
print('Frozen 96 complete 100-word DOT states, lossless storage bytes:',packed.storage_bytes)
