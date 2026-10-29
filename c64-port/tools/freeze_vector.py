#!/usr/bin/env python3
"""Freeze selected complete states after independent VAD/VSU equality."""
import hashlib,pathlib
root=pathlib.Path(__file__).resolve().parents[1]
data=(root/'build/vector-reference.snapshots').read_bytes()
assert data==(root/'build/vector-native.snapshots').read_bytes()
rows=[list(map(int,r.split())) for r in data.decode().splitlines()]
indices=[i*8 if i<32 else 256+i-32 if i<48 else 4496+i-48 for i in range(64)]
selected=[rows[2*n+sub] for n in indices for sub in range(2)]
assert len(selected)==128 and all(len(r)==53 for r in selected)
s=('/* SPDX-License-Identifier: MIT\n * GENERATED from unchanged assembly by tools/freeze_vector.py.\n'
   ' * Full snapshot SHA256: '+hashlib.sha256(data).hexdigest()+'\n */\n'
   '#ifndef VECTOR_VECTORS_H\n#define VECTOR_VECTORS_H\n#include <stdint.h>\n'
   'static const uint16_t vector_reference_words[128][50]={\n')
for r in selected: s+='    {'+','.join(str(v) for v in r[3:])+'}, /* %u/%u */\n'%tuple(r[:2])
s+='};\nstatic const uint16_t vector_reference_path[128]={\n'
for i in range(0,len(selected),16): s+='    '+','.join(str(r[2]) for r in selected[i:i+16])+',\n'
s+='};\n#endif\n';(root/'tests/vector_vectors.h').write_text(s)
print('Frozen 128 complete 50-word VAD/VSU states and 9-bit overflow observations')
