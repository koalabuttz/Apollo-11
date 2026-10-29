#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze complete source snapshots as lossless input-relative patches."""
import pathlib,subprocess,hashlib
R=pathlib.Path(__file__).resolve().parents[1]
core=['src/agc_word.c','src/dsky_relay.c','src/cm_relay.c','tests/cm_pinball_cases.c','tests/cm_relay_cases.c']
subprocess.run(['cc','-O2','-std=c99','-Iinclude','-Itests',*core,'tools/cm_relay_inputs.c','-o','build/cm-relay-inputs'],cwd=R,check=True)
inputs=subprocess.check_output(['./build/cm-relay-inputs'],cwd=R,text=True)
a=(R/'build/cm-relay-reference.snapshots').read_bytes();assert a==(R/'build/cm-relay-native.snapshots').read_bytes()
rows=[list(map(int,l.split()))for l in a.decode().splitlines()];before=[list(map(int,l.split()))for l in inputs.splitlines()];assert len(rows)==len(before)==4096 and all(len(r)==2054 for r in rows+before)
numbers=[k*61 for k in range(64)]
patch=[];offsets=[0]
for n in numbers:
 for f,(v,b) in enumerate(zip(rows[n][2:],before[n][2:])):
  if v!=b:patch.append((f,v))
 offsets.append(len(patch))
lines=['/* SPDX-License-Identifier: MIT',' * Full original CM source state, lossless deltas from independent inputs.',' * Snapshot SHA256 '+hashlib.sha256(a).hexdigest()+' */','#ifndef CM_RELAY_VECTORS_H','#define CM_RELAY_VECTORS_H','#include "cm_relay_cases.h"','static const uint16_t cm_relay_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t cm_relay_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t cm_relay_patch[][2]={']
lines+=[' {'+str(f)+','+str(v)+'},'for f,v in patch];lines+=['};','static const uint16_t cm_relay_results[]={'+','.join(str(rows[n][1])for n in numbers)+'};','#endif',''];(R/'tests/cm_relay_vectors.h').write_text('\n'.join(lines));print('Frozen64 complete2052-field original states; patches',len(patch))
