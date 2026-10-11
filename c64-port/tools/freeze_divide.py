#!/usr/bin/env python3
"""Freeze full final states and path observations after independent equality."""
import hashlib,pathlib
root=pathlib.Path(__file__).resolve().parents[1]
data=(root/'build/divide-reference.snapshots').read_bytes()
assert data==(root/'build/divide-native.snapshots').read_bytes()
rows=[list(map(int,r.split())) for r in data.decode().splitlines()]
assert len(rows)==8098 and all(len(r)==97 for r in rows)
selected=[r for r in rows if r[0]<147 and r[2]==1]; assert len(selected)==156
s=('/* SPDX-License-Identifier: MIT\n * GENERATED from original assembly using pinned hardware DV by tools/freeze_divide.py.\n'
   ' * Full snapshot SHA256: '+hashlib.sha256(data).hexdigest()+'\n */\n'
   '#ifndef DIVIDE_VECTORS_H\n#define DIVIDE_VECTORS_H\n#include <stdint.h>\n'
   'static const uint16_t divide_reference_words[156][93]={\n')
for r in selected: s+='    {'+','.join(str(v) for v in r[4:])+'}, /* %u/%u */\n'%tuple(r[:2])
s+='};\nstatic const uint8_t divide_reference_path[156]={\n'
for i in range(0,len(selected),16): s+='    '+','.join(str(r[3]) for r in selected[i:i+16])+',\n'
s+='};\n#endif\n'; (root/'tests/divide_vectors.h').write_text(s)
print('Frozen 156 complete final 93-word states and original division paths')
