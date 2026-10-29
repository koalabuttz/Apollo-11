#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,hashlib
R=pathlib.Path(__file__).resolve().parents[1];a=(R/'build/cm-idle-dispatch-reference.snapshots').read_bytes();assert a==(R/'build/cm-idle-dispatch-native.snapshots').read_bytes()
rows=[list(map(int,line.split())) for line in a.decode().splitlines()];assert len(rows)==4096 and all(len(row)==2056 for row in rows)
def fnv(row):
 h=2166136261
 for v in row:
  for b in [v&255,v>>8]:h=((h^b)*16777619)&0xffffffff
 return h
indices=[(j//32)*1024+((j%32)*37)%1024 for j in range(128)]
s='/* SPDX-License-Identifier: MIT\n * Original snapshot SHA256: '+hashlib.sha256(a).hexdigest()+' */\n#ifndef CM_IDLE_DISPATCH_VECTORS_H\n#define CM_IDLE_DISPATCH_VECTORS_H\n#include <stdint.h>\n#ifdef __mos__\nstatic const uint16_t cm_idle_dispatch_indices[128]={'+','.join(map(str,indices))+'};\nstatic const uint32_t cm_idle_dispatch_hash[128]={'+','.join(str(fnv(rows[n][1:]))+'u' for n in indices)+'};\n#else\nstatic const uint32_t cm_idle_dispatch_hash[4096]={'+','.join(str(fnv(row[1:]))+'u' for row in rows)+'};\n#endif\nstatic const uint16_t cm_idle_dispatch_sample_indices[2]={0,3072};\nstatic const uint16_t cm_idle_dispatch_samples[2][2055]={\n'+',\n'.join('{'+','.join(map(str,rows[n][1:]))+'}' for n in [0,3072])+'};\n#endif\n'
(R/'tests/cm_idle_dispatch_vectors.h').write_text(s);print('4096 source fingerprints; C64 128 representative fingerprints and two full states')
