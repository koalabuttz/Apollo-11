#!/usr/bin/env python3
"""Freeze complete final states after independent assembly equality."""
import hashlib,pathlib
root=pathlib.Path(__file__).resolve().parents[1]
data=(root/'build/push-reference.snapshots').read_bytes()
assert data==(root/'build/push-native.snapshots').read_bytes()
rows=[list(map(int,r.split())) for r in data.decode().splitlines()]
assert len(rows)==3496 and all(len(r)==89 for r in rows)
selected=[r for r in rows if r[0]<191 and r[2]==1]; assert len(selected)==200
s=('/* SPDX-License-Identifier: MIT\n * GENERATED from original assembly by tools/freeze_push.py.\n'
   ' * Full snapshot SHA256: '+hashlib.sha256(data).hexdigest()+'\n */\n'
   '#ifndef PUSH_VECTORS_H\n#define PUSH_VECTORS_H\n#include <stdint.h>\n'
   'static const uint16_t push_reference_words[200][86]={\n')
for r in selected: s+='    {'+','.join(str(v) for v in r[3:])+'}, /* %u/%u */\n'%tuple(r[:2])
s+='};\n#endif\n'; (root/'tests/push_vectors.h').write_text(s)
print('Frozen 200 complete final 86-word original-assembly states')
