#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,hashlib
R=pathlib.Path(__file__).resolve().parents[1];a=(R/'build/cm-restart-select-reference.snapshots').read_bytes();assert a==(R/'build/cm-restart-select-native.snapshots').read_bytes()
rows=[list(map(int,line.split())) for line in a.decode().splitlines()];assert len(rows)==6144 and all(len(row)==2050 for row in rows)
def fnv(row):
 h=2166136261
 for v in row:
  for b in [v&255,v>>8]:h=((h^b)*16777619)&0xffffffff
 return h
s='/* SPDX-License-Identifier: MIT\n * Source snapshot SHA256: '+hashlib.sha256(a).hexdigest()+' */\n#ifndef CM_RSSELECT_VECTORS_H\n#define CM_RSSELECT_VECTORS_H\n#include <stdint.h>\nstatic const uint32_t cm_rsselect_hash[6144]={\n'
s+=','.join(str(fnv(row[1:]))+'u' for row in rows)+'};\nstatic const uint16_t cm_rsselect_sample_indices[2]={0,4096};\nstatic const uint16_t cm_rsselect_samples[2][2049]={\n'
s+=',\n'.join('{'+','.join(map(str,rows[n][1:]))+'}' for n in [0,4096])+'};\n#endif\n'
(R/'tests/cm_restart_select_vectors.h').write_text(s);print('6144 full-image hashes, 2 complete 2049-value vectors')
