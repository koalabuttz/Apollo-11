#!/usr/bin/env python3
"""Lossless original full-image snapshots expressed as deltas from input data."""
import pathlib,hashlib,subprocess
root=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-std=c99','-Iinclude','-Itests','tests/p66_bias_cases.c','tests/dsky_pinball_cases.c','tests/dsky_pinball_zero_cases.c','tools/dsky_pinball_zero_inputs.c','-o','build/dsky-pinball-zero-inputs'],cwd=root,check=True)
inputs=subprocess.check_output(['./build/dsky-pinball-zero-inputs'],cwd=root,text=True)
a=(root/'build/dsky-pinball-zero-reference.snapshots').read_bytes();assert a==(root/'build/dsky-pinball-zero-native.snapshots').read_bytes()
rows=[list(map(int,l.split())) for l in a.decode().splitlines()];before=[list(map(int,l.split())) for l in inputs.splitlines()]
assert len(rows)==len(before)==4096 and all(len(r)==2052 for r in rows+before)
patch=[];offsets=[0];numbers=[n if n<48 else 128+(n-48)*229 for n in range(64)]
for n in numbers:
 for f,(v,b) in enumerate(zip(rows[n][2:],before[n][2:])):
  if v!=b:patch.append((f,v))
 offsets.append(len(patch))
lines=['/* SPDX-License-Identifier: MIT',' * Lossless complete original DSPIN/11DSPIN/2BLANK erasable images relative to input data.',' * Snapshot SHA256 '+hashlib.sha256(a).hexdigest()+' */','#ifndef DSKY_PINBALL_ZERO_VECTORS_H','#define DSKY_PINBALL_ZERO_VECTORS_H','#include "dsky_pinball_zero_cases.h"','static const uint16_t dsky_pinball_zero_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t dsky_pinball_zero_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t dsky_pinball_zero_patch[][2]={']
lines+=[' {'+str(f)+','+str(v)+'},' for f,v in patch];lines+=['};','static const uint16_t dsky_pinball_zero_results[]={'+','.join(str(rows[n][1]) for n in numbers)+'};','#endif','']
(root/'tests/dsky_pinball_zero_vectors.h').write_text('\n'.join(lines))
print('Frozen64 complete2050wordimages, patches:',len(patch),'packed fixturebytes:',len(patch)*4+65*2+64*2)
