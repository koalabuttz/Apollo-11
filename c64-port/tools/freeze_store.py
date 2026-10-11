#!/usr/bin/env python3
"""Freeze full final states and path observations after independent equality."""
import hashlib,pathlib
root=pathlib.Path(__file__).resolve().parents[1]
data=(root/'build/store-reference.snapshots').read_bytes()
assert data==(root/'build/store-native.snapshots').read_bytes()
rows=[list(map(int,r.split())) for r in data.decode().splitlines()]
assert len(rows)==5044 and all(len(r)==93 for r in rows)
extras=[353,438,523,608]
selected=[r for r in rows if r[0]<163 and r[2]==1]
selected += [next(r for r in rows if r[0]==n and r[2]==1) for n in extras]
assert len(selected)==176
s=('/* SPDX-License-Identifier: MIT\n * GENERATED from original assembly by tools/freeze_store.py.\n'
   ' * Full snapshot SHA256: '+hashlib.sha256(data).hexdigest()+'\n */\n'
   '#ifndef STORE_VECTORS_H\n#define STORE_VECTORS_H\n#include <stdint.h>\n'
   'static const uint16_t store_reference_words[176][89]={\n')
for r in selected: s+='    {'+','.join(str(v) for v in r[4:])+'}, /* %u/%u */\n'%tuple(r[:2])
s+='};\nstatic const uint8_t store_reference_path[176]={\n'
for i in range(0,len(selected),16): s+='    '+','.join(str(r[3]) for r in selected[i:i+16])+',\n'
s+='};\n#endif\n'; (root/'tests/store_vectors.h').write_text(s)
print('Frozen 176 complete final 89-word states and original skip paths')
