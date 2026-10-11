#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,hashlib
R=pathlib.Path(__file__).resolve().parents[1];a=(R/'build/cm-waitlist-extended-reference.snapshots').read_bytes();assert a==(R/'build/cm-waitlist-extended-native.snapshots').read_bytes()
rows=[list(map(int,line.split())) for line in a.decode().splitlines()];assert len(rows)==2048 and all(len(row)==2052 for row in rows)
def fnv(row):
 h=2166136261
 for v in row:
  for b in [v&255,v>>8]:h=((h^b)*16777619)&0xffffffff
 return h
s='/* SPDX-License-Identifier: MIT\n * Source snapshot SHA256: '+hashlib.sha256(a).hexdigest()+' */\n#ifndef CM_WAITLIST_EXTENDED_VECTORS_H\n#define CM_WAITLIST_EXTENDED_VECTORS_H\n#include <stdint.h>\nstatic const uint32_t cm_waitlist_extended_hash[2048]={\n'
s+=','.join(str(fnv(row[1:]))+'u' for row in rows)+'};\nstatic const uint16_t cm_waitlist_extended_sample_indices[2]={0,1024};\nstatic const uint16_t cm_waitlist_extended_samples[2][2051]={\n'
s+=',\n'.join('{'+','.join(map(str,rows[n][1:]))+'}' for n in [0,1024])+'};\n#endif\n'
(R/'tests/cm_waitlist_extended_vectors.h').write_text(s);print('2048 full-image hashes, 2 complete 2051-value vectors')
