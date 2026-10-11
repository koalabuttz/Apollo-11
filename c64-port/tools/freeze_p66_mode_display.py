#!/usr/bin/env python3
"""Lossless original full-image snapshots expressed as deltas from input data."""
import pathlib,hashlib,subprocess
root=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-std=c99','-Iinclude','-Itests','tests/p66_bias_cases.c','tests/p66_mode_display_cases.c','tools/p66_mode_display_inputs.c','-o','build/p66-mode-display-inputs'],cwd=root,check=True)
inputs=subprocess.check_output(['./build/p66-mode-display-inputs'],cwd=root,text=True)
a=(root/'build/p66-mode-display-reference.snapshots').read_bytes();assert a==(root/'build/p66-mode-display-native.snapshots').read_bytes()
rows=[list(map(int,l.split())) for l in a.decode().splitlines()];before=[list(map(int,l.split())) for l in inputs.splitlines()]
assert len(rows)==len(before)==1024 and all(len(r)==2052 for r in rows+before)
patch=[];offsets=[0];numbers=[(n//8)*104+[0,1,9,10,66,99,100,102][n%8] for n in range(64)]
for n in numbers:
 for f,(v,b) in enumerate(zip(rows[n][2:],before[n][2:])):
  if v!=b:patch.append((f,v))
 offsets.append(len(patch))
lines=['/* SPDX-License-Identifier: MIT',' * Lossless complete original DSPMMJB erasable images relative to input data.',' * Snapshot SHA256 '+hashlib.sha256(a).hexdigest()+' */','#ifndef P66_MODE_VECTORS_H','#define P66_MODE_VECTORS_H','#include "p66_mode_display_cases.h"','static const uint16_t p66_mode_display_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t p66_mode_display_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t p66_mode_display_patch[][2]={']
lines+=[' {'+str(f)+','+str(v)+'},' for f,v in patch];lines+=['};','static const uint16_t p66_mode_display_results[]={'+','.join(str(rows[n][1]) for n in numbers)+'};','#endif','']
(root/'tests/p66_mode_display_vectors.h').write_text('\n'.join(lines))
print('Frozen64 complete2050wordimages, patches:',len(patch),'packed fixturebytes:',len(patch)*4+65*2+64*2)
