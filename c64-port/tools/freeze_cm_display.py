#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze complete source snapshots as lossless input-relative patches."""
import pathlib,subprocess,hashlib
R=pathlib.Path(__file__).resolve().parents[1]
core=['src/agc_word.c','src/cm_executive.c','src/cm_dispatch.c','src/cm_native_image.c','src/cm_basic_priority.c','src/cm_init_display.c','tests/cm_display_cases.c']
subprocess.run(['cc','-O2','-std=c99','-Iinclude','-Itests',*core,'tools/cm_display_inputs.c','-o','build/cm-display-inputs'],cwd=R,check=True)
inputs=subprocess.check_output(['./build/cm-display-inputs'],cwd=R,text=True)
a=(R/'build/cm-display-reference.snapshots').read_bytes();assert a==(R/'build/cm-display-native.snapshots').read_bytes()
rows=[list(map(int,l.split()))for l in a.decode().splitlines()];before=[list(map(int,l.split()))for l in inputs.splitlines()];assert len(rows)==len(before)==12288 and all(len(r)==2052 for r in rows+before)
numbers=[128*k+(k*29)%128 for k in range(32)]+[4096+128*k+(k*31)%128 for k in range(16)]+[8192+128*k+(k*7)%128 for k in range(16)]
patch=[];offsets=[0]
for n in numbers:
 for f,(v,b) in enumerate(zip(rows[n][2:],before[n][2:])):
  if v!=b:patch.append((f,v))
 offsets.append(len(patch))
lines=['/* SPDX-License-Identifier: MIT',' * Full original CM source state, lossless deltas from independent inputs.',' * Snapshot SHA256 '+hashlib.sha256(a).hexdigest()+' */','#ifndef CM_DISPLAY_VECTORS_H','#define CM_DISPLAY_VECTORS_H','#include "cm_display_cases.h"','static const uint16_t cm_display_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t cm_display_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t cm_display_patch[][2]={']
lines+=[' {'+str(f)+','+str(v)+'},'for f,v in patch];lines+=['};','static const uint16_t cm_display_results[]={'+','.join(str(rows[n][1])for n in numbers)+'};','#endif',''];(R/'tests/cm_display_vectors.h').write_text('\n'.join(lines));print('Frozen64 complete2050-field original states; patches',len(patch))
