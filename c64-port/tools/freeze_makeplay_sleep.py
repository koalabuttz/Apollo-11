#!/usr/bin/env python3
"""Lossless original full-image snapshots expressed as deltas from input data."""
import pathlib,hashlib,subprocess
root=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-std=c99','-Iinclude','-Itests','src/agc_word.c','src/p66_makeplay.c','src/p66_makeplay_sleep.c','tests/throttle_bridge_cases.c','tests/makeplay_cases.c','tests/makeplay_sleep_cases.c','tools/makeplay_sleep_inputs.c','-o','build/makeplay-sleep-inputs'],cwd=root,check=True)
inputs=subprocess.check_output(['./build/makeplay-sleep-inputs'],cwd=root,text=True)
a=(root/'build/makeplay-sleep-reference.snapshots').read_bytes();assert a==(root/'build/makeplay-sleep-native.snapshots').read_bytes()
rows=[list(map(int,l.split())) for l in a.decode().splitlines()];before=[list(map(int,l.split())) for l in inputs.splitlines()]
assert len(rows)==len(before)==16384 and all(len(r)==2052 for r in rows+before)
patch=[];offsets=[0];numbers=[g*4096+n for g in range(4) for n in list(range(8))+[127,511,1023,1535,2047,2559,3071,4095]]
for n in numbers:
 for f,(v,b) in enumerate(zip(rows[n][2:],before[n][2:])):
  if v!=b:patch.append((f,v))
 offsets.append(len(patch))
lines=['/* SPDX-License-Identifier: MIT',' * Lossless complete original THROTTLE BRIDGES erasable images relative to input data.',' * Snapshot SHA256 '+hashlib.sha256(a).hexdigest()+' */','#ifndef MAKEPLAY_SLEEP_VECTORS_H','#define MAKEPLAY_SLEEP_VECTORS_H','#include "makeplay_sleep_cases.h"','static const uint16_t makeplay_sleep_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t makeplay_sleep_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t makeplay_sleep_patch[][2]={']
lines+=[' {'+str(f)+','+str(v)+'},' for f,v in patch];lines+=['};','static const uint16_t makeplay_sleep_results[]={'+','.join(str(rows[n][1]) for n in numbers)+'};','#endif','']
(root/'tests/makeplay_sleep_vectors.h').write_text('\n'.join(lines))
print('Frozen64 complete2050wordimages, patches:',len(patch),'packed fixturebytes:',len(patch)*4+65*2+64*2)
