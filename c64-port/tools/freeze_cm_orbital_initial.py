#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Lossless source deltas against the verified deterministic input constructor."""
import pathlib,array,hashlib
from frozen_words import encode_rows,c_header
root=pathlib.Path(__file__).resolve().parents[1];data=(root/'build/cm-orbital-initial-reference.snapshots').read_bytes();assert data==(root/'build/cm-orbital-initial-native.snapshots').read_bytes();raw=array.array('H');raw.frombytes(data);assert len(raw)==5120*2051
initial={};previous={};groups={p:{}for p in range(4)}
for k in range(0,len(raw),2051):
 n,p=raw[k:k+2];w=list(raw[k+2:k+2051])
 if p==255:initial[n]=w
 else:groups[p][n]=(previous[n],w)
 previous[n]=w
s='/* SPDX-License-Identifier: MIT\n * Every source erasable word represented losslessly by original input seed plus\n * exact address/value changes; unchanged words remain in the expected image.\n * Source complete-image SHA256:'+hashlib.sha256(data).hexdigest()+'\n * Input-constructor SHA256:'+hashlib.sha256((root/'tests/cm_orbital_initial_cases.c').read_bytes()).hexdigest()+'\n */\n#ifndef CM_RECTIFY_VECTORS_H\n#define CM_RECTIFY_VECTORS_H\n'
for phase in range(4):
 indices=list(range(12))+[127,128,255,256]+[257+31*i for i in range(8)]+[768+31*i for i in range(8)]
 rows=[]
 for n in indices:
  before,after=groups[phase][n];entry=[(i,x)for i,x in enumerate(before)if x!=initial[n][i]];exit=[(i,x)for i,x in enumerate(after)if x!=before[i]]
  row=[n,len(entry)]+[v for pair in entry for v in pair]+[len(exit)]+[v for pair in exit for v in pair];rows.append(row)
 width=max(map(len,rows));rows=[row+[0]*(width-len(row))for row in rows]
 packed=encode_rows(rows);s+=f'#if !defined(CM_RECTIFY_PHASE_SELECTED) || CM_RECTIFY_PHASE_SELECTED=={phase}\n'+c_header(packed,f'cm_orbital_initial_phase{phase}')+'#endif\n';print(phase,'32 full2048word+IRQ source entry/exit pairs',packed.storage_bytes,'packed delta bytes')
s+='#endif\n';(root/'tests/cm_orbital_initial_vectors.h').write_text(s)
