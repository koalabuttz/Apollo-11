#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Lossless complete-image source entry/exit deltas, not native expected values."""
import pathlib,array,hashlib
from frozen_words import encode_rows,c_header
root=pathlib.Path(__file__).resolve().parents[1];data=(root/'build/cm-difeq-reference.snapshots').read_bytes();assert data==(root/'build/cm-difeq-native.snapshots').read_bytes();raw=array.array('H');raw.frombytes(data);assert len(raw)==19840*2050
initial={};previous={};groups={}
for k in range(0,len(raw),2050):
 n,p=raw[k:k+2];w=list(raw[k+2:k+2050])
 if p==255:initial[n]=w
 else:groups.setdefault(((n//512 if n<1536 else (n-1536)//128),p),{})[n]=(previous[n],w)
 previous[n]=w
s='/* SPDX-License-Identifier: MIT\n * Full physical source images represented losslessly by input seed plus exact\n * before/after address/value deltas; every unchanged word remains checked.\n * Source complete-image SHA256:'+hashlib.sha256(data).hexdigest()+'\n * Input-constructor SHA256:'+hashlib.sha256((root/'tests/cm_difeq_cases.c').read_bytes()).hexdigest()+'\n */\n#ifndef CM_DIFEQ_VECTORS_H\n#define CM_DIFEQ_VECTORS_H\n'
for (stage,phase),cases in sorted(groups.items()):
 indices=[stage*512+n for n in list(range(12))+[127,128,255,256]+[257+31*i for i in range(8)]+[480+4*i for i in range(8)]]
 if stage<2:indices[-4:]=[1536+stage*128+n for n in [0,1,5,65]]
 rows=[]
 for n in indices:
  before,after=cases[n];entry=[(i,x)for i,x in enumerate(before)if x!=initial[n][i]];exit=[(i,x)for i,x in enumerate(after)if x!=before[i]]
  rows.append([n,len(entry)]+[v for pair in entry for v in pair]+[len(exit)]+[v for pair in exit for v in pair])
 width=max(map(len,rows));rows=[r+[0]*(width-len(r))for r in rows];packed=encode_rows(rows);selector=stage*16+phase
 s+=f'#if !defined(CM_DIFEQ_SELECTOR) || CM_DIFEQ_SELECTOR=={selector}\n'+c_header(packed,f'cm_difeq_stage{stage}_phase{phase}')+'#endif\n';print(stage,phase,'32 complete source image entry/exit pairs',packed.storage_bytes,'bytes')
s+='#endif\n';(root/'tests/cm_difeq_vectors.h').write_text(s)
