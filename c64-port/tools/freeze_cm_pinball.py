#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze complete source snapshots as lossless input-relative patches."""
import pathlib,subprocess,hashlib
R=pathlib.Path(__file__).resolve().parents[1]
core=['src/agc_word.c','src/dsky_pinball.c','src/cm_pinball.c','tests/cm_pinball_cases.c']
subprocess.run(['cc','-O2','-std=c99','-Iinclude','-Itests',*core,'tools/cm_pinball_inputs.c','-o','build/cm-pinball-inputs'],cwd=R,check=True)
inputs=subprocess.check_output(['./build/cm-pinball-inputs'],cwd=R,text=True)
a=(R/'build/cm-pinball-reference.snapshots').read_bytes();assert a==(R/'build/cm-pinball-native.snapshots').read_bytes()
rows=[list(map(int,l.split()))for l in a.decode().splitlines()];before=[list(map(int,l.split()))for l in inputs.splitlines()];assert len(rows)==len(before)==8192 and all(len(r)==2052 for r in rows+before)
numbers=list(range(24))+list(range(48,72))+[128+k*503 for k in range(16)]
patch=[];offsets=[0]
for n in numbers:
 for f,(v,b) in enumerate(zip(rows[n][2:],before[n][2:])):
  if v!=b:patch.append((f,v))
 offsets.append(len(patch))
lines=['/* SPDX-License-Identifier: MIT',' * Full original CM source state, lossless deltas from independent inputs.',' * Snapshot SHA256 '+hashlib.sha256(a).hexdigest()+' */','#ifndef CM_PINBALL_VECTORS_H','#define CM_PINBALL_VECTORS_H','#include "cm_pinball_cases.h"','static const uint16_t cm_pinball_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t cm_pinball_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t cm_pinball_patch[][2]={']
lines+=[' {'+str(f)+','+str(v)+'},'for f,v in patch];lines+=['};','static const uint16_t cm_pinball_results[]={'+','.join(str(rows[n][1])for n in numbers)+'};','#endif',''];(R/'tests/cm_pinball_vectors.h').write_text('\n'.join(lines));print('Frozen64 complete2050-field original states; patches',len(patch))
