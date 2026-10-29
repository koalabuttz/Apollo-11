#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze independent source entry/exit pairs for native overlay phases."""
import pathlib,hashlib,collections
from frozen_words import encode_rows,c_header
root=pathlib.Path(__file__).resolve().parents[1]
for kind,count in [('unit',3),('radial',9),('projection',3)]:
 data=(root/f'build/{kind}-phased-reference.snapshots').read_bytes()
 assert data==(root/f'build/{kind}-phased-native.snapshots').read_bytes()
 source=[list(map(int,x.split()))for x in data.decode().splitlines()]
 pairs=collections.defaultdict(list)
 if kind=='radial':
  prev={}
  for row in source:
   n,step,stage,status=row[:4];key=(n,step)
   if stage:pairs[stage-1].append((n,step,prev[key],row[3:]))
   prev[key]=row[4:]
 else:
  inputs={r[0]:r[3:]for r in [list(map(int,x.split()))for x in (root/f'build/{kind}-phased-reference.inputs').read_text().splitlines()]}
  prev=dict(inputs)
  for row in source:
   n,phase,status=row[:3];pairs[phase].append((n,0,prev[n],row[2:]));prev[n]=row[3:]
 s=f'/* SPDX-License-Identifier: MIT\n * Independent original source entry/exit states, SHA256 {hashlib.sha256(data).hexdigest()}.\n */\n#ifndef {kind.upper()}_PHASED_VECTORS_H\n#define {kind.upper()}_PHASED_VECTORS_H\n'
 for phase in range(count):
  candidates=pairs[phase]; chosen=[];seen=set()
  def add(j):
   if j not in seen:chosen.append(j);seen.add(j)
  # First corners plus every observed status and end-of-corpus regressions.
  for j in range(min(12,len(candidates))):add(j)
  for status in sorted(set(r[3][0]for r in candidates)):
   for j,r in enumerate(candidates):
    if r[3][0]==status:add(j);break
  for j in range(max(0,len(candidates)-4),len(candidates)):add(j)
  j=0
  while len(chosen)<32:
   add((j*len(candidates)//17)%len(candidates));j+=1
   if j>1000:
    for k in range(len(candidates)):add(k)
    break
  chosen=chosen[:32];assert len(chosen)==32
  packed=encode_rows([candidates[j][2]+candidates[j][3]for j in chosen]);prefix=f'{kind}_phase{phase}'
  s+=f'#if !defined(GUIDANCE_PHASE_SELECTED) || GUIDANCE_PHASE_SELECTED=={phase}\n'+c_header(packed,prefix)+'#endif\n'
  print(kind,phase,'32 full source entry/exit pairs',packed.storage_bytes,'packed bytes','statuses',sorted(set(candidates[j][3][0]for j in chosen)))
 s+='#endif\n';(root/f'tests/{kind}_phased_vectors.h').write_text(s)
