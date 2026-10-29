#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import hashlib
from pathlib import Path
from frozen_words import c_header,encode_rows
root=Path(__file__).resolve().parents[1];data=(root/'build/radial-reference.snapshots').read_bytes()
assert data==(root/'build/radial-native.snapshots').read_bytes()
rows=[list(map(int,r.split()))for r in data.decode().splitlines()]
selected=list(range(163))+[173,250,269,282,301,378,397,405,406,409,410,413,414,417,418]+list(range(419,427))+list(range(1443,1451))
for suffix,indices in [('all',selected)]+[(chr(97+i),selected[35*i:35*(i+1)])for i in range(6)]:
    subset=[r for r in rows if r[0] in indices and r[2]==1]
    packed=encode_rows([r[3:]for r in subset]);guard='RADIAL_VECTORS_'+suffix.upper()+'_H'
    text='/* SPDX-License-Identifier: MIT\n * GENERATED lossless original assembly/hardware-DV state. SHA256:'+hashlib.sha256(data).hexdigest()+' */\n#ifndef '+guard+'\n#define '+guard+'\n'
    text+=c_header(packed,'radial')
    inputs=[r for r in rows if r[0] in indices and r[2]==0]
    input_packed=encode_rows([r[4:] for r in inputs])
    text+=c_header(input_packed,'radial_input')
    text+='#define RA_REFERENCE_CASES '+str(len(indices))+'u\nstatic const uint16_t radial_indices[RA_REFERENCE_CASES]={'+','.join(map(str,indices))+'};\n#endif\n'
    (root/f'tests/radial_vectors_{suffix}.h').write_text(text)
    print(suffix,len(indices),len(subset),packed.storage_bytes,input_packed.storage_bytes)
wrapper='/* SPDX-License-Identifier: MIT */\n#ifndef RADIAL_VECTORS_H\n#define RADIAL_VECTORS_H\n'
for i in range(6):
 wrapper+=('#if' if not i else '#elif')+' defined(RADIAL_DIAGNOSTIC_HALF) && RADIAL_DIAGNOSTIC_HALF=='+str(i)+'\n#include \"radial_vectors_'+chr(97+i)+'.h\"\n'
wrapper+='#else\n#include \"radial_vectors_all.h\"\n#endif\n#endif\n'
(root/'tests/radial_vectors.h').write_text(wrapper)
