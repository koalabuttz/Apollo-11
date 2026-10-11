#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,hashlib,subprocess
from frozen_words import encode_rows,c_header
root=pathlib.Path(__file__).resolve().parents[1]
data=(root/'build/acceleration-store-reference.snapshots').read_bytes();assert data==(root/'build/acceleration-store-native.snapshots').read_bytes();states=[list(map(int,r.split()))for r in data.decode().splitlines()];assert len(states)==81920
indices=list(range(16))+[4096+i*127 for i in range(8)]+[8192+i*1023 for i in range(8)]
s='/* SPDX-License-Identifier: MIT\n * Original source phase entry/exit states; SHA256:'+hashlib.sha256(data).hexdigest()+' */\n#ifndef ACCELERATION_STORE_VECTORS_H\n#define ACCELERATION_STORE_VECTORS_H\n'
for phase in range(2):
 rows=[]
 for n in indices:
  before=states[n*5+phase+2][3:];after=states[n*5+phase+3];assert after[:3]==[n,phase+3,0];rows.append(before+after[2:])
 packed=encode_rows(rows);s+=f'#if !defined(ACCELERATION_STORE_SELECTED) || ACCELERATION_STORE_SELECTED=={phase}\n'+c_header(packed,f'acceleration_store{phase}')+'#endif\n';print('Phase',phase,'32 original entry/exit states;',packed.storage_bytes,'packed bytes')
s+='#endif\n';(root/'tests/acceleration_store_vectors.h').write_text(s)
