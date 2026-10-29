#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma
R=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-Iinclude','-Itests','src/agc_word.c','tests/dsky_read_pair_cases.c','tools/dsky_read_pair_inputs.c','-o','build/dsky-read-pair-inputs'],cwd=R,check=True)
selected=[g*8192+n for g in range(2)for n in [36,37,44,45]+[k*33 for k in range(60)]]
selected.sort()
proc=subprocess.Popen(['build/dsky-read-pair-inputs'],cwd=R,stdout=subprocess.PIPE,text=True)
inputs={}
for line in proc.stdout:
 row=line.split();n=int(row[0])
 if n in selected:inputs[n]=list(map(int,row[1:]))
assert proc.wait()==0
ao=[0];ap=[]
with lzma.open(R/'build/dsky-read-pair-reference.snapshots.xz','rt')as f:
 for line in f:
  row=list(map(int,line.split()));n=row[0]
  if n in selected:
   ap.extend((i,v)for i,(v,b)in enumerate(zip(row[1:],inputs[n]))if v!=b)
   ao.append(len(ap))
assert len(ao)==129
lines=['/* SPDX-License-Identifier: MIT */','#ifndef DSKY_READ_PAIR_VECTORS_H','#define DSKY_READ_PAIR_VECTORS_H','static const uint16_t dsky_read_pair_numbers[]={'+','.join(map(str,selected))+'};','static const uint16_t dsky_read_pair_after_offsets[]={'+','.join(map(str,ao))+'};','static const uint16_t dsky_read_pair_after_patch[][2]={']+[' {'+str(i)+','+str(v)+'},'for i,v in ap]+['};','#endif',''];(R/'tests/dsky_read_pair_vectors.h').write_text('\n'.join(lines));print('Frozen128 full original fresh READLO pair states;',len(ap),'source output patches')
