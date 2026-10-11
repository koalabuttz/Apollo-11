#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma
R=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-Iinclude','-Itests','src/agc_word.c','tests/dsky_decimal_raw_cases.c','tests/dsky_decimal_general_cases.c','tools/dsky_decimal_raw_inputs.c','-o','build/dsky-decimal-raw-inputs'],cwd=R,check=True)
selected=[g*131072+k for g in range(2)for k in list(range(16))+list(range(65520,65552))+list(range(131056,131072))]
selected.sort()
wanted=set(selected);inputs={}
proc=subprocess.Popen(['build/dsky-decimal-raw-inputs'],cwd=R,stdout=subprocess.PIPE,text=True)
for line in proc.stdout:
 n=int(line.split(' ',1)[0])
 if n in wanted:inputs[n]=list(map(int,line.split()))[1:]
assert proc.wait()==0
ao=[0];ap=[]
with lzma.open(R/'build/dsky-decimal-raw-reference.snapshots.xz','rt')as f:
 for line in f:
  row=list(map(int,line.split()));n=row[0]
  if n in selected:
   ap.extend((i,v)for i,(v,b)in enumerate(zip(row[1:],inputs[n]))if v!=b)
   ao.append(len(ap))
assert len(ao)==129
lines=['/* SPDX-License-Identifier: MIT */','#ifndef DSKY_DECIMAL_RAW_VECTORS_H','#define DSKY_DECIMAL_RAW_VECTORS_H','static const uint32_t dsky_decimal_raw_numbers[]={'+','.join(map(str,selected))+'};','static const uint16_t dsky_decimal_raw_after_offsets[]={'+','.join(map(str,ao))+'};','static const uint16_t dsky_decimal_raw_after_patch[][2]={']+[' {'+str(i)+','+str(v)+'},'for i,v in ap]+['};','#endif',''];(R/'tests/dsky_decimal_raw_vectors.h').write_text('\n'.join(lines));print('Frozen128 full original decimal formatter caller/CCSHOLE states;',len(ap),'source output patches')
