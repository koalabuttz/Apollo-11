#!/usr/bin/env python3
"""Freeze complete numeric states after independent original-assembly match."""
import pathlib,hashlib
root=pathlib.Path(__file__).resolve().parents[1]
data=(root/'build/scale-reference.snapshots').read_bytes()
assert data==(root/'build/scale-native.snapshots').read_bytes()
rows=[list(map(int,r.split())) for r in data.decode().splitlines()]
assert len(rows)==3364 and all(len(r)==43 for r in rows)
selected=[r for r in rows if r[0]<283]; assert len(selected)==292
s=('/* SPDX-License-Identifier: MIT\n * GENERATED from original assembly by tools/freeze_scale.py.\n'
   ' * Full snapshot SHA256: '+hashlib.sha256(data).hexdigest()+'\n */\n'
   '#ifndef SCALE_VECTORS_H\n#define SCALE_VECTORS_H\n#include <stdint.h>\n'
   'static const uint16_t scale_reference_words[292][41]={\n')
for r in selected: s+='    {'+','.join(str(v) for v in r[2:])+'}, /* %u/%u */\n'%tuple(r[:2])
s+='};\n#endif\n'; (root/'tests/scale_vectors.h').write_text(s)
print('Frozen 292 complete 41-word original-assembly states')
