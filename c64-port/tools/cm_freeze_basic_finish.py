#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,hashlib
R=pathlib.Path(__file__).resolve().parents[1];a=(R/'build/cm-basic-finish-reference.snapshots').read_bytes();assert a==(R/'build/cm-basic-finish-native.snapshots').read_bytes()
rows=[list(map(int,line.split())) for line in a.decode().splitlines()];assert len(rows)==16384 and all(len(row)==2056 for row in rows)
def fnv(row):
 h=2166136261
 for v in row:
  for b in [v&255,v>>8]:h=((h^b)*16777619)&0xffffffff
 return h
indices=[(j//64)*8192+((j%64)*131)%8192 for j in range(128)]
s='/* SPDX-License-Identifier: MIT\n * Original snapshot SHA256: '+hashlib.sha256(a).hexdigest()+' */\n#ifndef CM_BASIC_FINISH_VECTORS_H\n#define CM_BASIC_FINISH_VECTORS_H\n#include <stdint.h>\n#ifdef __mos__\nstatic const uint16_t cm_basic_finish_indices[128]={'+','.join(map(str,indices))+'};\nstatic const uint32_t cm_basic_finish_hash[128]={'+','.join(str(fnv(rows[n][1:]))+'u' for n in indices)+'};\n#else\nstatic const uint32_t cm_basic_finish_hash[16384]={'+','.join(str(fnv(row[1:]))+'u' for row in rows)+'};\n#endif\nstatic const uint16_t cm_basic_finish_sample_indices[2]={0,8192};\nstatic const uint16_t cm_basic_finish_samples[2][2055]={\n'+',\n'.join('{'+','.join(map(str,rows[n][1:]))+'}' for n in [0,8192])+'};\n#endif\n'
(R/'tests/cm_basic_finish_vectors.h').write_text(s);print('16384 source fingerprints; C64 128 representative fingerprints and two full states')
