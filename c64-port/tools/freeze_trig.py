#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import hashlib,pathlib
from frozen_words import encode_rows,c_header
root=pathlib.Path(__file__).resolve().parents[1];data=(root/'build/trig-reference.snapshots').read_bytes()
assert data==(root/'build/trig-native.snapshots').read_bytes()
rows=[list(map(int,r.split()))for r in data.decode().splitlines()]
indices=list(range(16))+[16,32,64,80,128,160,256,512,1024,1280,2048,3072,4095,4096,6000,8191]
selected=[rows[4*n+op]for n in indices for op in range(4)]
packed=encode_rows([r[2:]for r in selected]);text='/* SPDX-License-Identifier: MIT\n * GENERATED complete unchanged assembly state; SHA256:'+hashlib.sha256(data).hexdigest()+' */\n#ifndef TRIG_VECTORS_H\n#define TRIG_VECTORS_H\n'
text+=c_header(packed,'trig')+'static const uint16_t trig_reference_indices[32]={'+','.join(map(str,indices))+'};\n#endif\n';(root/'tests/trig_vectors.h').write_text(text)
print('Frozen128 complete130word states lossless bytes:',packed.storage_bytes)
