#!/usr/bin/env python3
"""Lossless original full-image snapshots expressed as deltas from input data."""
import pathlib,hashlib,subprocess
root=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-std=c99','-Iinclude','-Itests','src/agc_word.c','src/cm_clockjob.c','tests/cm_trig_cases.c','tests/cm_clockjob_cases.c','tools/cm_clockjob_inputs.c','-o','build/cm-clockjob-inputs'],cwd=root,check=True)
inputs=subprocess.check_output(['./build/cm-clockjob-inputs'],cwd=root,text=True)
a=(root/'build/cm-clockjob-reference.snapshots').read_bytes();assert a==(root/'build/cm-clockjob-native.snapshots').read_bytes()
rows=[list(map(int,l.split())) for l in a.decode().splitlines()];before=[list(map(int,l.split())) for l in inputs.splitlines()]
assert len(rows)==len(before)==24576 and all(len(r)==2052 for r in rows+before)
patch=[];offsets=[0];numbers=list(range(8))+[128+n*503 for n in range(16)]+[8192+n*503 for n in range(16)]+[16384+n for n in range(8)]+[16512+n*503 for n in range(16)]
for n in numbers:
 for f,(v,b) in enumerate(zip(rows[n][2:],before[n][2:])):
  if v!=b:patch.append((f,v))
 offsets.append(len(patch))
lines=['/* SPDX-License-Identifier: MIT',' * Lossless complete original CM CLOCKJOB erasable images relative to input data.',' * Snapshot SHA256 '+hashlib.sha256(a).hexdigest()+' */','#ifndef CM_CLOCKJOB_VECTORS_H','#define CM_CLOCKJOB_VECTORS_H','#include "cm_clockjob_cases.h"','static const uint16_t cm_clockjob_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t cm_clockjob_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t cm_clockjob_patch[][2]={']
lines+=[' {'+str(f)+','+str(v)+'},' for f,v in patch];lines+=['};','static const uint16_t cm_clockjob_results[]={'+','.join(str(rows[n][1]) for n in numbers)+'};','#endif','']
(root/'tests/cm_clockjob_vectors.h').write_text('\n'.join(lines))
print('Frozen64 complete2050wordimages, patches:',len(patch),'packed fixturebytes:',len(patch)*4+65*2+64*2)
