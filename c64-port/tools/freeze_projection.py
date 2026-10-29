#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import hashlib,pathlib
from frozen_words import encode_rows,c_header
root=pathlib.Path(__file__).resolve().parents[1];data=(root/'build/projection-reference.snapshots').read_bytes();inputs=(root/'build/projection-reference.inputs').read_bytes()
assert data==(root/'build/projection-native.snapshots').read_bytes() and inputs==(root/'build/projection-native.inputs').read_bytes()
rows=[list(map(int,r.split()))for r in data.decode().splitlines()];entry=[list(map(int,r.split()))for r in inputs.decode().splitlines()]
indices=list(range(16))+[16,32,64,80,128,160,256,512,1024,1280,2048,3072,4095,4096,6000,8191]+list(range(4100,4116))
for suffix,index in [('all',indices)]+[(chr(97+i),indices[12*i:12*(i+1)])for i in range(4)]:
 packed=encode_rows([rows[3*n+op][2:]for n in index for op in range(3)]);seed=encode_rows([entry[n][3:]for n in index]);guard='PROJECTION_VECTORS_'+suffix.upper()+'_H'
 text='/* SPDX-License-Identifier: MIT\n * GENERATED complete unchanged assembly/hardware-DV state; SHA256:'+hashlib.sha256(data).hexdigest()+' */\n#ifndef '+guard+'\n#define '+guard+'\n'+c_header(packed,'projection')+c_header(seed,'projection_input')
 text+='#define PJ_REFERENCE_GROUPS '+str(len(index))+'u\nstatic const uint16_t projection_indices[PJ_REFERENCE_GROUPS]={'+','.join(map(str,index))+'};\n#endif\n';(root/f'tests/projection_vectors_{suffix}.h').write_text(text);print(suffix,len(index),packed.storage_bytes,seed.storage_bytes)
text='/* SPDX-License-Identifier: MIT */\n#ifndef PROJECTION_VECTORS_H\n#define PROJECTION_VECTORS_H\n'
for i in range(4):text+=('#if' if not i else '#elif')+' defined(PROJECTION_DIAGNOSTIC_PART) && PROJECTION_DIAGNOSTIC_PART=='+str(i)+'\n#include "projection_vectors_'+chr(97+i)+'.h"\n'
text+='#else\n#include "projection_vectors_all.h"\n#endif\n#endif\n';(root/'tests/projection_vectors.h').write_text(text)
