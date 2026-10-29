#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma
R=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-Iinclude','-Itests','src/agc_word.c','tests/dsky_nv_blank_cases.c','tools/dsky_nv_blank_inputs.c','-o','build/dsky-nv-blank-inputs'],cwd=R,check=True)
selected=[group*1024+n for group in range(8)for n in [0,1,2,3,4,5,6,7,9,17,25,33,41,49,57,1023]]
inputs={int(row.split()[0]):list(map(int,row.split()))[1:]for row in subprocess.check_output(['build/dsky-nv-blank-inputs'],cwd=R,text=True).splitlines()if int(row.split()[0])in selected}
ao=[0];ap=[];count=0
with lzma.open(R/'build/dsky-nv-blank-reference.snapshots.xz','rt')as f:
 for line in f:
  row=list(map(int,line.split()));n=row[0]
  if n in selected:
   ap.extend((i,v)for i,(v,b)in enumerate(zip(row[1:],inputs[n]))if v!=b)
   ao.append(len(ap));count+=1
assert count==128
lines=['/* SPDX-License-Identifier: MIT */','#ifndef DSKY_NV_BLANK_VECTORS_H','#define DSKY_NV_BLANK_VECTORS_H','static const uint16_t dsky_nv_blank_numbers[]={'+','.join(map(str,selected))+'};','static const uint16_t dsky_nv_blank_after_offsets[]={'+','.join(map(str,ao))+'};','static const uint16_t dsky_nv_blank_after_patch[][2]={']+[' {'+str(i)+','+str(v)+'},'for i,v in ap]+['};','#endif',''];(R/'tests/dsky_nv_blank_vectors.h').write_text('\n'.join(lines));print('Frozen128 full original BLANKDSP/FLASHOFF states;',len(ap),'source output patches')
