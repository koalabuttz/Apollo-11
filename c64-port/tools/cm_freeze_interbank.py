#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,hashlib
R=pathlib.Path(__file__).resolve().parents[1];a=(R/'build/cm-interbank-reference.snapshots').read_bytes();assert a==(R/'build/cm-interbank-native.snapshots').read_bytes()
rows=[list(map(int,line.split())) for line in a.decode().splitlines()];assert len(rows)==7168 and all(len(row)==2052 for row in rows)
def fnv(row):
 h=2166136261
 for v in row:
  for b in [v&255,v>>8]:h=((h^b)*16777619)&0xffffffff
 return h
indices=[(j//8)*512+((j%8)*67)%512 for j in range(112)]
s='/* SPDX-License-Identifier: MIT\n * Original snapshot SHA256: '+hashlib.sha256(a).hexdigest()+' */\n#ifndef CM_INTERBANK_VECTORS_H\n#define CM_INTERBANK_VECTORS_H\n#include <stdint.h>\n#ifdef __mos__\nstatic const uint16_t cm_interbank_indices[112]={'+','.join(map(str,indices))+'};\nstatic const uint32_t cm_interbank_hash[112]={'+','.join(str(fnv(rows[n][1:]))+'u' for n in indices)+'};\n#else\nstatic const uint32_t cm_interbank_hash[7168]={'+','.join(str(fnv(row[1:]))+'u' for row in rows)+'};\n#endif\nstatic const uint16_t cm_interbank_sample_indices[2]={0,3072};\nstatic const uint16_t cm_interbank_samples[2][2051]={\n'+',\n'.join('{'+','.join(map(str,rows[n][1:]))+'}' for n in [0,3072])+'};\n#endif\n'
(R/'tests/cm_interbank_vectors.h').write_text(s);print('7168 source fingerprints; C64 112 representative fingerprints and two full states')
