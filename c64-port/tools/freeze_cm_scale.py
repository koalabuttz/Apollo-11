#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze complete source snapshots as lossless input-relative patches."""
import pathlib,subprocess,hashlib
R=pathlib.Path(__file__).resolve().parents[1]
core=['src/agc_word.c','src/agc_multiword.c','src/guidance_product.c','src/dsky_scale.c','src/cm_scale.c','tests/p66_bias_cases.c','tests/dsky_scale_cases.c','tests/cm_scale_cases.c']
subprocess.run(['cc','-O2','-std=c99','-Iinclude','-Itests',*core,'tools/cm_scale_inputs.c','-o','build/cm-scale-inputs'],cwd=R,check=True)
inputs=subprocess.check_output(['./build/cm-scale-inputs'],cwd=R,text=True)
a=(R/'build/cm-scale-reference.snapshots').read_bytes();assert a==(R/'build/cm-scale-native.snapshots').read_bytes()
rows=[list(map(int,l.split()))for l in a.decode().splitlines()];before=[list(map(int,l.split()))for l in inputs.splitlines()];assert len(rows)==len(before)==4096 and all(len(r)==2052 for r in rows+before)
numbers=list(range(32))+[64+k*124 for k in range(32)]
patch=[];offsets=[0]
for n in numbers:
 for f,(v,b) in enumerate(zip(rows[n][2:],before[n][2:])):
  if v!=b:patch.append((f,v))
 offsets.append(len(patch))
lines=['/* SPDX-License-Identifier: MIT',' * Full original CM source state, lossless deltas from independent inputs.',' * Snapshot SHA256 '+hashlib.sha256(a).hexdigest()+' */','#ifndef CM_SCALE_VECTORS_H','#define CM_SCALE_VECTORS_H','#include "cm_scale_cases.h"','static const uint16_t cm_scale_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t cm_scale_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t cm_scale_patch[][2]={']
lines+=[' {'+str(f)+','+str(v)+'},'for f,v in patch];lines+=['};','static const uint16_t cm_scale_results[]={'+','.join(str(rows[n][1])for n in numbers)+'};','#endif',''];(R/'tests/cm_scale_vectors.h').write_text('\n'.join(lines));print('Frozen64 complete2050-field original states; patches',len(patch))
