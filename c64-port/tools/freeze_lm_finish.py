#!/usr/bin/env python3
"""Lossless original full-image snapshots expressed as deltas from input data."""
import pathlib,hashlib,subprocess
root=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-std=c99','-Iinclude','-Itests','src/agc_word.c','tests/throttle_bridge_cases.c','tests/lm_priority_cases.c','tests/lm_sleep_cases.c','tests/lm_finish_cases.c','tools/lm_finish_inputs.c','-o','build/lm-finish-inputs'],cwd=root,check=True)
inputs=subprocess.check_output(['./build/lm-finish-inputs'],cwd=root,text=True)
a=(root/'build/lm-finish-reference.snapshots').read_bytes();assert a==(root/'build/lm-finish-native.snapshots').read_bytes()
rows=[list(map(int,l.split())) for l in a.decode().splitlines()];before=[list(map(int,l.split())) for l in inputs.splitlines()]
assert len(rows)==len(before)==16384 and all(len(r)==2053 for r in rows+before)
patch=[];offsets=[0];numbers=list(range(8))+[128+n*503 for n in range(16)]+[8192+n*503 for n in range(16)]+[256+n*37 for n in range(24)]
for n in numbers:
 for f,(v,b) in enumerate(zip(rows[n][2:],before[n][2:])):
  if v!=b:patch.append((f,v))
 offsets.append(len(patch))
lines=['/* SPDX-License-Identifier: MIT',' * Lossless complete original ENDOFJOB erasable images relative to input data.',' * Snapshot SHA256 '+hashlib.sha256(a).hexdigest()+' */','#ifndef LM_FINISH_VECTORS_H','#define LM_FINISH_VECTORS_H','#include "lm_finish_cases.h"','static const uint16_t lm_finish_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t lm_finish_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t lm_finish_patch[][2]={']
lines+=[' {'+str(f)+','+str(v)+'},' for f,v in patch];lines+=['};','static const uint16_t lm_finish_results[]={'+','.join(str(rows[n][1]) for n in numbers)+'};','#endif','']
(root/'tests/lm_finish_vectors.h').write_text('\n'.join(lines))
print('Frozen64 complete2051wordimages, patches:',len(patch),'packed fixturebytes:',len(patch)*4+65*2+64*2)
