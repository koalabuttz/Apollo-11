#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import hashlib,pathlib
r=pathlib.Path(__file__).resolve().parents[1];data=(r/'build/hardware-divide-reference.snapshots').read_bytes();assert data==(r/'build/hardware-divide-native.snapshots').read_bytes();rows=[list(map(int,x.split())) for x in data.decode().splitlines()]
indices=list(range(0,4096,17))+[4095,4096,4097,69630,69631,69632,69633,135166,135167,135168,135169,262143,327678,327679]
s='/* SPDX-License-Identifier: MIT\n * Frozen pinned hardware-DV adapter states; SHA256:'+hashlib.sha256(data).hexdigest()+' */\n#ifndef HARDWARE_DIVIDE_VECTORS_H\n#define HARDWARE_DIVIDE_VECTORS_H\n#define HARDWARE_DIVIDE_ROWS '+str(len(indices))+'u\nstatic const uint32_t hardware_divide_indices[HARDWARE_DIVIDE_ROWS]={'+','.join(map(str,indices))+'};\nstatic const uint16_t hardware_divide_expected[HARDWARE_DIVIDE_ROWS][2]={'+','.join('{'+','.join(map(str,rows[i][1:]))+'}' for i in indices)+'};\n#endif\n';(r/'tests/hardware_divide_vectors.h').write_text(s);print(len(indices),'complete register pairs frozen')
