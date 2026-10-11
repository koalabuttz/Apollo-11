#!/usr/bin/env python3
"""Freeze selected complete states after independent VXSC/DMP1 equality."""
import hashlib,pathlib
root=pathlib.Path(__file__).resolve().parents[1]
data=(root/'build/product-reference.snapshots').read_bytes()
assert data==(root/'build/product-native.snapshots').read_bytes()
rows=[list(map(int,r.split())) for r in data.decode().splitlines()]
indices=[282 if i==62 else 395 if i==63 else i*8 if i<32 else 256+i-32 if i<48 else 3072+i-48 for i in range(64)]
selected=[rows[2*n+sub] for n in indices for sub in range(2)]
assert len(selected)==128 and all(len(r)==52 for r in selected)
s=('/* SPDX-License-Identifier: MIT\n * GENERATED from unchanged assembly by tools/freeze_product.py.\n'
   ' * Full snapshot SHA256: '+hashlib.sha256(data).hexdigest()+'\n */\n'
   '#ifndef PRODUCT_VECTORS_H\n#define PRODUCT_VECTORS_H\n#include <stdint.h>\n'
   'static const uint16_t product_reference_words[128][50]={\n')
for r in selected: s+='    {'+','.join(str(v) for v in r[2:])+'}, /* %u/%u */\n'%tuple(r[:2])
s+='};\n#endif\n';(root/'tests/product_vectors.h').write_text(s)
print('Frozen 128 complete 50-word scalar DMP1/vector VXSC states')
