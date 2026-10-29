#!/usr/bin/env python3
"""Freeze complete states only after exact independent original-assembly match."""
import pathlib,hashlib
root=pathlib.Path(__file__).resolve().parents[1]
data=(root/'build/prepare-reference.snapshots').read_bytes()
assert data==(root/'build/prepare-native.snapshots').read_bytes()
rows=[list(map(int,r.split())) for r in data.decode().splitlines()]
assert len(rows)==3396 and all(len(r)==43 for r in rows)
selected=[r for r in rows if r[0]<147]; assert len(selected)==324
s=('/* SPDX-License-Identifier: MIT\n * GENERATED from original assembly by tools/freeze_prepare.py.\n'
   ' * Full snapshot SHA256: '+hashlib.sha256(data).hexdigest()+'\n */\n'
   '#ifndef PREPARE_VECTORS_H\n#define PREPARE_VECTORS_H\n#include <stdint.h>\n'
   'static const uint16_t prepare_reference_words[324][39]={\n')
for r in selected: s+='    {'+','.join(str(v) for v in r[4:])+'}, /* %u/%u/%u */\n'%tuple(r[:3])
s+='};\nstatic const uint8_t prepare_reference_mask[324]={\n'
for i in range(0,len(selected),16): s+='    '+','.join(str(r[3]) for r in selected[i:i+16])+',\n'
s+='};\n#endif\n'; (root/'tests/prepare_vectors.h').write_text(s)
print('Frozen 324 complete 39-word states and original instruction-skip observations')
