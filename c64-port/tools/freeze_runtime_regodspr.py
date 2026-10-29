#!/usr/bin/env python3
"""Lossless original full-image snapshots expressed as deltas from input data."""
import pathlib,hashlib,subprocess
root=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-std=c99','-Iinclude','-Itests','tests/throttle_bridge_cases.c','tests/display_control_cases.c','tests/regodspr_cases.c','tests/runtime_regodspr_cases.c','tools/runtime_regodspr_inputs.c','-o','build/runtime_regodspr-inputs'],cwd=root,check=True)
inputs=subprocess.check_output(['./build/runtime_regodspr-inputs'],cwd=root,text=True)
a=(root/'build/runtime_regodspr-reference.snapshots').read_bytes();assert a==(root/'build/runtime_regodspr-native.snapshots').read_bytes()
rows=[list(map(int,l.split())) for l in a.decode().splitlines()];before=[list(map(int,l.split())) for l in inputs.splitlines()]
assert len(rows)==len(before)==8192 and all(len(r)==2052 for r in rows+before)
patch=[];offsets=[0];numbers=list(range(64));numbers[0]=64;numbers[1]=4097;numbers[32]=4096
for n in numbers:
 for f,(v,b) in enumerate(zip(rows[n][2:],before[n][2:])):
  if v!=b:patch.append((f,v))
 offsets.append(len(patch))
lines=['/* SPDX-License-Identifier: MIT',' * Lossless complete original THROTTLE BRIDGES erasable images relative to input data.',' * Snapshot SHA256 '+hashlib.sha256(a).hexdigest()+' */','#ifndef RUNTIME_REGODSPR_VECTORS_H','#define RUNTIME_REGODSPR_VECTORS_H','#include "runtime_regodspr_cases.h"','static const uint16_t runtime_regodspr_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t runtime_regodspr_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t runtime_regodspr_patch[][2]={']
lines+=[' {'+str(f)+','+str(v)+'},' for f,v in patch];lines+=['};','static const uint16_t runtime_regodspr_results[]={'+','.join(str(rows[n][1]) for n in numbers)+'};','#endif','']
(root/'tests/runtime_regodspr_vectors.h').write_text('\n'.join(lines))
print('Frozen64 complete2050wordimages, patches:',len(patch),'packed fixturebytes:',len(patch)*4+65*2+64*2)
