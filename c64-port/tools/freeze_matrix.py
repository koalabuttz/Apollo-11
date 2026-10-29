#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma,hashlib
R=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-std=c99','-Iinclude','-Itests','src/agc_word.c','tests/matrix_cases.c','tools/matrix_inputs.c','-o','build/matrix-inputs'],cwd=R,check=True)
inputs=[0,1,2,3,7,16,31,127,128,129,255,256,1023,2047,4094,4095];numbers=[n*8+(i%2)*4+p for i,n in enumerate(inputs)for p in range(4)];patch=[];offsets=[0];hashes=[]
for mission in ['LM','CM']:
 proc=subprocess.Popen(['build/matrix-inputs',mission],cwd=R,stdout=subprocess.PIPE);h=hashlib.sha256()
 with lzma.open(R/f'build/matrix-{mission}-reference.snapshots.xz','rb')as f:
  for n,line in enumerate(f):
   h.update(line);before=proc.stdout.readline()
   if n not in numbers:continue
   row=list(map(int,line.split()));old=list(map(int,before.split()));assert len(row)==len(old)==2052
   for field,(v,b)in enumerate(zip(row[2:],old[2:])):
    if v!=b:patch.append((field,v))
   offsets.append(len(patch))
 assert n==32767 and len(offsets)==(65 if mission=='LM' else 129)and proc.wait()==0
 hashes.append(h.hexdigest())
lines=['/* SPDX-License-Identifier: MIT',' * Complete source LM/CM MXV/VXM states: '+','.join(hashes)+' */','#ifndef MATRIX_VECTORS_H','#define MATRIX_VECTORS_H','#include "matrix_cases.h"','static const uint16_t matrix_numbers[]={'+','.join(map(str,numbers))+'};','static const uint16_t matrix_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t matrix_patch[][2]={']+[' {'+str(f)+','+str(v)+'},'for f,v in patch]+['};','#endif','']
(R/'tests/matrix_vectors.h').write_text('\n'.join(lines));print('Frozen128 complete2050 source states,64 eachmission;',len(patch),'word patches')
