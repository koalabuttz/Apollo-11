#!/usr/bin/env python3
"""Freeze 128 whole ENEMA states using lossless baseline + word replacements."""
import hashlib,pathlib
root=pathlib.Path(__file__).resolve().parents[1]
a=(root/'build/enema-reference.snapshots').read_bytes()
assert a==(root/'build/enema-native.snapshots').read_bytes()
rows=[list(map(int,line.split())) for line in a.decode().splitlines()]
assert len(rows)==4096 and all(len(row)==459 for row in rows)
base=[r[2:] for r in rows[:16]]
selected=[rows[n*31] for n in range(128)]
patch=[]; offsets=[0]
for row in selected:
 for field,(expected,b) in enumerate(zip(row[2:],base[row[0]%16])):
  if expected!=b: patch.append((field,expected))
 offsets.append(len(patch))
lines=['/* SPDX-License-Identifier: MIT',' * Original unchanged ENEMA whole-state snapshots, losslessly represented.',' * Source snapshot SHA256 '+hashlib.sha256(a).hexdigest()+' */','#ifndef ENEMA_VECTORS_H','#define ENEMA_VECTORS_H','#include "enema_cases.h"','#define ENEMA_TARGET_COUNT 128','static const agc_word enema_base[16][ENEMA_WORD_COUNT] = {']
lines+=[' {'+','.join(map(str,row))+'},' for row in base]
lines+=['};','static const uint16_t enema_patch_offset[129]={'+','.join(map(str,offsets))+'};','static const uint16_t enema_patch[][2]={']
lines+=[' {'+str(field)+','+str(value)+'},' for field,value in patch]
lines+=['};','static const agc_word enema_result[128]={'+','.join(str(row[1]) for row in selected)+'};','#endif','']
(root/'tests/enema_vectors.h').write_text('\n'.join(lines))
print('Frozen128 complete457word ENEMA states, baseline words:',16*457,'patches:',len(patch),'packed bytes:',16*457*2+len(patch)*4+129*2+128*2)
