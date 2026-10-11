#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze complete source snapshots as lossless input-relative patches."""
import pathlib,subprocess,hashlib
R=pathlib.Path(__file__).resolve().parents[1]
core=['src/agc_word.c','src/agc_multiword.c','src/dsky_pinball.c','src/dsky_charin_number.c','src/cm_charin_number.c','tests/p66_bias_cases.c','tests/dsky_nv_key_cases.c','tests/dsky_charin_number_cases.c','tests/cm_charin_number_cases.c']
subprocess.run(['cc','-O2','-std=c99','-Iinclude','-Itests',*core,'tools/cm_charin_number_inputs.c','-o','build/cm-charin-number-inputs'],cwd=R,check=True)
inputs=subprocess.check_output(['./build/cm-charin-number-inputs'],cwd=R,text=True)
a=(R/'build/cm-charin-number-reference.snapshots').read_bytes();assert a==(R/'build/cm-charin-number-native.snapshots').read_bytes()
rows=[list(map(int,l.split()))for l in a.decode().splitlines()];before=[list(map(int,l.split()))for l in inputs.splitlines()];assert len(rows)==len(before)==8192 and all(len(r)==2054 for r in rows+before)
numbers=list(range(16))+list(range(4096,4115))+[4115,4120,4125,4134,4139,4144]+list(range(7680,7690))+[4229,4234,4239,4286,4291,4296,4343,4348,4353,4400,4405,4410,4457]
patch=[];offsets=[0]
for n in numbers:
 for f,(v,b) in enumerate(zip(rows[n][2:],before[n][2:])):
  if v!=b:patch.append((f,v))
 offsets.append(len(patch))
lines=['/* SPDX-License-Identifier: MIT',' * Full original CM source state, lossless deltas from independent inputs.',' * Snapshot SHA256 '+hashlib.sha256(a).hexdigest()+' */','#ifndef CM_CHARIN_NUMBER_VECTORS_H','#define CM_CHARIN_NUMBER_VECTORS_H','#include "cm_charin_number_cases.h"','static const uint16_t cm_charin_number_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t cm_charin_number_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t cm_charin_number_patch[][2]={']
lines+=[' {'+str(f)+','+str(v)+'},'for f,v in patch];lines+=['};','static const uint16_t cm_charin_number_results[]={'+','.join(str(rows[n][1])for n in numbers)+'};','#endif',''];(R/'tests/cm_charin_number_vectors.h').write_text('\n'.join(lines));print('Frozen64 complete2052-field original states; patches',len(patch))
