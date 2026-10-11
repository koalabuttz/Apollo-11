#!/usr/bin/env python3
"""Freeze full final states and path observations after independent equality."""
import hashlib,pathlib
root=pathlib.Path(__file__).resolve().parents[1]
data=(root/'build/elapsed-reference.snapshots').read_bytes()
assert data==(root/'build/elapsed-native.snapshots').read_bytes()
rows=[list(map(int,r.split())) for r in data.decode().splitlines()]
assert len(rows)==4664 and all(len(r)==93 for r in rows)
selected=[r for r in rows if r[0]<163 and r[2]==1]; assert len(selected)==172
s=('/* SPDX-License-Identifier: MIT\n * GENERATED from original assembly by tools/freeze_elapsed.py.\n'
   ' * Full snapshot SHA256: '+hashlib.sha256(data).hexdigest()+'\n */\n'
   '#ifndef ELAPSED_VECTORS_H\n#define ELAPSED_VECTORS_H\n#include <stdint.h>\n'
   'static const uint16_t elapsed_reference_words[172][89]={\n')
for r in selected: s+='    {'+','.join(str(v) for v in r[4:])+'}, /* %u/%u */\n'%tuple(r[:2])
s+='};\nstatic const uint8_t elapsed_reference_path[172]={\n'
for i in range(0,len(selected),16): s+='    '+','.join(str(r[3]) for r in selected[i:i+16])+',\n'
s+='};\n#endif\n'; (root/'tests/elapsed_vectors.h').write_text(s)
print('Frozen 172 complete final 89-word states and original skip paths')
