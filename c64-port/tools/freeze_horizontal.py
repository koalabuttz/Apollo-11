#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import hashlib,pathlib
from frozen_words import encode_rows,c_header
root=pathlib.Path(__file__).resolve().parents[1];data=(root/'build/horizontal-reference.snapshots').read_bytes();inputs=(root/'build/horizontal-reference.inputs').read_bytes()
assert data==(root/'build/horizontal-native.snapshots').read_bytes() and inputs==(root/'build/horizontal-native.inputs').read_bytes()
rows=[list(map(int,r.split()))for r in data.decode().splitlines()];entry=[list(map(int,r.split()))for r in inputs.decode().splitlines()]
indices=list(range(16))+[16,32,64,80,128,160,256,512,1024,1280,2048,3072,4095,4096,6000,8191]
for suffix,index in [('all',indices)]+[(chr(97+i),indices[8*i:8*(i+1)])for i in range(4)]:
 packed=encode_rows([rows[8*n+op][2:]for n in index for op in range(8)]);seed=encode_rows([entry[n][3:]for n in index]);guard='HORIZONTAL_VECTORS_'+suffix.upper()+'_H'
 text='/* SPDX-License-Identifier: MIT\n * GENERATED complete unchanged assembly/hardware-DV state; SHA256:'+hashlib.sha256(data).hexdigest()+' */\n#ifndef '+guard+'\n#define '+guard+'\n'+c_header(packed,'horizontal')+c_header(seed,'horizontal_input')
 text+='#define HH_REFERENCE_GROUPS '+str(len(index))+'u\nstatic const uint16_t horizontal_indices[HH_REFERENCE_GROUPS]={'+','.join(map(str,index))+'};\n#endif\n';(root/f'tests/horizontal_vectors_{suffix}.h').write_text(text);print(suffix,len(index),packed.storage_bytes,seed.storage_bytes)
text='/* SPDX-License-Identifier: MIT */\n#ifndef HORIZONTAL_VECTORS_H\n#define HORIZONTAL_VECTORS_H\n'
for i in range(4):text+=('#if' if not i else '#elif')+' defined(HORIZONTAL_DIAGNOSTIC_PART) && HORIZONTAL_DIAGNOSTIC_PART=='+str(i)+'\n#include "horizontal_vectors_'+chr(97+i)+'.h"\n'
text+='#else\n#include "horizontal_vectors_all.h"\n#endif\n#endif\n';(root/'tests/horizontal_vectors.h').write_text(text)
