#!/usr/bin/env python3
"""Lossless original full-image snapshots expressed as deltas from input data."""
import pathlib,hashlib,subprocess
root=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-std=c99','-Iinclude','-Itests','tests/throttle_bridge_cases.c','src/agc_word.c','tests/goplay_cases.c','tools/goplay_inputs.c','-o','build/goplay-inputs'],cwd=root,check=True)
inputs=subprocess.check_output(['./build/goplay-inputs'],cwd=root,text=True)
a=(root/'build/goplay-reference.snapshots').read_bytes();assert a==(root/'build/goplay-native.snapshots').read_bytes()
rows=[list(map(int,l.split())) for l in a.decode().splitlines()];before=[list(map(int,l.split())) for l in inputs.splitlines()]
assert len(rows)==len(before)==8192 and all(len(r)==2054 for r in rows+before)
patch=[];offsets=[0];numbers=[n*127*2 for n in range(32)]+[n*127*2+1 for n in range(32)]
for n in numbers:
 for f,(v,b) in enumerate(zip(rows[n][3:],before[n][3:])):
  if v!=b:patch.append((f,v))
 offsets.append(len(patch))
lines=['/* SPDX-License-Identifier: MIT',' * Lossless complete original THROTTLE BRIDGES erasable images relative to input data.',' * Snapshot SHA256 '+hashlib.sha256(a).hexdigest()+' */','#ifndef GOPLAY_VECTORS_H','#define GOPLAY_VECTORS_H','#include "goplay_cases.h"','static const uint16_t goplay_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t goplay_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t goplay_patch[][2]={']
lines+=[' {'+str(f)+','+str(v)+'},' for f,v in patch];lines+=['};','static const uint16_t goplay_results[]={'+','.join(str(rows[n][2]) for n in numbers)+'};','#endif','']
(root/'tests/goplay_vectors.h').write_text('\n'.join(lines))
print('Frozen64 complete2051wordimages, patches:',len(patch),'packed fixturebytes:',len(patch)*4+65*2+64*2)
