#!/usr/bin/env python3
"""Lossless original full-image snapshots expressed as deltas from input data."""
import pathlib,hashlib,subprocess
root=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-std=c99','-Iinclude','-Itests','tests/throttle_bridge_cases.c','src/agc_word.c','src/p66_nv_entry.c','tests/nv_entry_cases.c','tests/nv_words_cases.c','tests/nv_rows_cases.c','src/p66_nv_words.c','src/dsky_two_digit.c','src/dsky_pinball.c','src/agc_multiword.c','tools/nv_rows_inputs.c','-o','build/nv-rows-inputs'],cwd=root,check=True)
inputs=subprocess.check_output(['./build/nv-rows-inputs'],cwd=root,text=True)
a=(root/'build/nv-rows-reference.snapshots').read_bytes();assert a==(root/'build/nv-rows-native.snapshots').read_bytes()
rows=[list(map(int,l.split())) for l in a.decode().splitlines()];before=[list(map(int,l.split())) for l in inputs.splitlines()]
assert len(rows)==len(before)==4096 and all(len(r)==2053 for r in rows+before)
patch=[];offsets=[0];numbers=[n*63 for n in range(64)]
for n in numbers:
 for f,(v,b) in enumerate(zip(rows[n][3:],before[n][3:])):
  if v!=b:patch.append((f,v))
 offsets.append(len(patch))
lines=['/* SPDX-License-Identifier: MIT',' * Lossless complete original THROTTLE BRIDGES erasable images relative to input data.',' * Snapshot SHA256 '+hashlib.sha256(a).hexdigest()+' */','#ifndef NV_ROWS_VECTORS_H','#define NV_ROWS_VECTORS_H','#include "nv_rows_cases.h"','static const uint16_t nv_rows_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t nv_rows_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t nv_rows_patch[][2]={']
lines+=[' {'+str(f)+','+str(v)+'},' for f,v in patch];lines+=['};','static const uint16_t nv_rows_results[]={'+','.join(str(rows[n][2]) for n in numbers)+'};','#endif','']
(root/'tests/nv_rows_vectors.h').write_text('\n'.join(lines))
print('Frozen64 complete2050wordimages, patches:',len(patch),'packed fixturebytes:',len(patch)*4+65*2+64*2)
