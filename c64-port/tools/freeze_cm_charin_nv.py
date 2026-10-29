#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze complete source snapshots as lossless input-relative patches."""
import pathlib,subprocess,hashlib
R=pathlib.Path(__file__).resolve().parents[1]
core=['src/agc_word.c','src/agc_multiword.c','src/dsky_pinball.c','src/dsky_nv_key.c','src/dsky_charin_nv.c','src/cm_charin_nv.c','tests/p66_bias_cases.c','tests/dsky_nv_key_cases.c','tests/dsky_charin_nv_cases.c','tests/cm_charin_nv_cases.c']
subprocess.run(['cc','-O2','-std=c99','-Iinclude','-Itests',*core,'tools/cm_charin_nv_inputs.c','-o','build/cm-charin-nv-inputs'],cwd=R,check=True)
inputs=subprocess.check_output(['./build/cm-charin-nv-inputs'],cwd=R,text=True)
a=(R/'build/cm-charin-nv-reference.snapshots').read_bytes();assert a==(R/'build/cm-charin-nv-native.snapshots').read_bytes()
rows=[list(map(int,l.split()))for l in a.decode().splitlines()];before=[list(map(int,l.split()))for l in inputs.splitlines()];assert len(rows)==len(before)==4096 and all(len(r)==2054 for r in rows+before)
numbers=list(range(32))+[128+(n-32)*123 for n in range(32,64)]
patch=[];offsets=[0]
for n in numbers:
 for f,(v,b) in enumerate(zip(rows[n][2:],before[n][2:])):
  if v!=b:patch.append((f,v))
 offsets.append(len(patch))
lines=['/* SPDX-License-Identifier: MIT',' * Full original CM source state, lossless deltas from independent inputs.',' * Snapshot SHA256 '+hashlib.sha256(a).hexdigest()+' */','#ifndef CM_CHARIN_NV_VECTORS_H','#define CM_CHARIN_NV_VECTORS_H','#include "cm_charin_nv_cases.h"','static const uint16_t cm_charin_nv_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t cm_charin_nv_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t cm_charin_nv_patch[][2]={']
lines+=[' {'+str(f)+','+str(v)+'},'for f,v in patch];lines+=['};','static const uint16_t cm_charin_nv_results[]={'+','.join(str(rows[n][1])for n in numbers)+'};','#endif',''];(R/'tests/cm_charin_nv_vectors.h').write_text('\n'.join(lines));print('Frozen64 complete2052-field original states; patches',len(patch))
