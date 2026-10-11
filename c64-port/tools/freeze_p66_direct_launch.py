#!/usr/bin/env python3
"""Lossless original full-image snapshots expressed as deltas from input data."""
import pathlib,hashlib,subprocess
root=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-std=c99','-Iinclude','-Itests','tests/p66_bias_cases.c','tests/p66_vertical_start_cases.c','tests/p66_launch_cases.c','tests/p66_direct_launch_cases.c','tools/p66_direct_launch_inputs.c','-o','build/p66-direct-launch-inputs'],cwd=root,check=True)
inputs=subprocess.check_output(['./build/p66-direct-launch-inputs'],cwd=root,text=True)
a=(root/'build/p66-direct-launch-reference.snapshots').read_bytes();assert a==(root/'build/p66-direct-launch-native.snapshots').read_bytes()
rows=[list(map(int,l.split())) for l in a.decode().splitlines()];before=[list(map(int,l.split())) for l in inputs.splitlines()]
assert len(rows)==len(before)==2048 and all(len(r)==2052 for r in rows+before)
patch=[];offsets=[0];numbers=list(range(8))+[1024+64*k+k%8 for k in range(16)]
for n in numbers:
 for f,(v,b) in enumerate(zip(rows[n][2:],before[n][2:])):
  if v!=b:patch.append((f,v))
 offsets.append(len(patch))
lines=['/* SPDX-License-Identifier: MIT',' * Lossless complete original STARTP66 erasable images relative to input data.',' * Snapshot SHA256 '+hashlib.sha256(a).hexdigest()+' */','#ifndef P66_DIRECT_LAUNCH_VECTORS_H','#define P66_DIRECT_LAUNCH_VECTORS_H','#include "p66_direct_launch_cases.h"','static const uint16_t p66_direct_launch_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t p66_direct_launch_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t p66_direct_launch_patch[][2]={']
lines+=[' {'+str(f)+','+str(v)+'},' for f,v in patch];lines+=['};','static const uint16_t p66_direct_launch_results[]={'+','.join(str(rows[n][1]) for n in numbers)+'};','#endif','']
(root/'tests/p66_direct_launch_vectors.h').write_text('\n'.join(lines))
print('Frozen24 complete2050wordimages, patches:',len(patch),'packed fixturebytes:',len(patch)*4+25*2+24*2)
