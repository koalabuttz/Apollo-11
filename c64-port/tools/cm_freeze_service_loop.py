#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,hashlib
R=pathlib.Path(__file__).resolve().parents[1];a=(R/'build/cm-service-loop-reference.snapshots').read_bytes();assert a==(R/'build/cm-service-loop-native.snapshots').read_bytes()
rows=[list(map(int,line.split())) for line in a.decode().splitlines()];assert all(len(r)==2054 for r in rows)
cases=[[] for n in range(384)]
for row in rows:assert row[1]==len(cases[row[0]]);cases[row[0]].append(row)
assert all(1<=len(c)<=6 for c in cases)
def fnv(row):
 h=2166136261
 for v in row:
  for b in [v&255,v>>8]:h=((h^b)*16777619)&0xffffffff
 return h
s='/* SPDX-License-Identifier: MIT\n * Original continuous snapshot SHA256: '+hashlib.sha256(a).hexdigest()+' */\n#ifndef CM_SERVICE_LOOP_VECTORS_H\n#define CM_SERVICE_LOOP_VECTORS_H\n#include <stdint.h>\n'
s+='static const uint8_t cm_service_loop_count[384]={'+','.join(str(len(c)) for c in cases)+'};\n'
s+='#ifdef __mos__\nstatic const uint32_t cm_service_loop_hash[64][6]={\n'+',\n'.join('{'+','.join(str(fnv(r[2:]))+'u' for r in cases[(run%6)*64+(run//6)*5])+'}' for run in range(64))+'};\n#else\n'
s+='static const uint32_t cm_service_loop_hash[384][6]={\n'+',\n'.join('{'+','.join(str(fnv(r[2:]))+'u' for r in c)+'}' for c in cases)+'};\n#endif\n'
s+='static const uint16_t cm_service_loop_sample[2052]={'+','.join(map(str,cases[0][-1][2:]))+'};\n#endif\n'
(R/'tests/cm_service_loop_vectors.h').write_text(s);print(len(rows),'whole-image checkpoints across384 continuous original cases, one full2052-value state')
