#!/usr/bin/env python3
"""Freeze complete sums after independent native/original equality."""
import hashlib,pathlib
from frozen_words import encode_rows,c_header
root=pathlib.Path(__file__).resolve().parents[1]
data=(root/'build/sum-reference.snapshots').read_bytes()
assert data==(root/'build/sum-native.snapshots').read_bytes()
rows=[list(map(int,line.split()))for line in data.decode().splitlines()]
assert len(rows)==27200 and all(len(row)==53 for row in rows)
indices=[index*8 if index<32 else 256+index-32 if index<48 else 4496+index-48 for index in range(64)]
selected=[rows[4*n+operation]for n in indices for operation in range(4)]
s=('/* SPDX-License-Identifier: MIT\n * GENERATED from unchanged assembly by tools/freeze_sum.py.\n'
   ' * Full snapshot SHA256: '+hashlib.sha256(data).hexdigest()+'\n */\n'
   '#ifndef SUM_VECTORS_H\n#define SUM_VECTORS_H\n')
packed=encode_rows([row[3:]for row in selected]);s+=c_header(packed,'sum')
s+='static const uint16_t sum_reference_path[256]={\n'
for index in range(0,len(selected),16):s+='    '+','.join(str(row[2])for row in selected[index:index+16])+',\n'
s+='};\n#endif\n';(root/'tests/sum_vectors.h').write_text(s)
print('Frozen 256 complete 50-word states and overflow paths; lossless word-storage bytes:',packed.storage_bytes)
