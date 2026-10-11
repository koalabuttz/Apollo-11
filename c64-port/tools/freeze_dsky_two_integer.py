#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma
R=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-Iinclude','-Itests','src/agc_word.c','tests/dsky_two_integer_cases.c','tools/dsky_two_integer_inputs.c','-o','build/dsky-two-integer-inputs'],cwd=R,check=True)
proc=subprocess.Popen(['build/dsky-two-integer-inputs'],cwd=R,stdout=subprocess.PIPE,text=True)
inputs={}
for row in proc.stdout:inputs[int(row.split()[0])]=list(map(int,row.split()))[1:]
assert proc.wait()==0
candidates=[[[],[]]for _ in range(5)]
previous=None;lastn=-1
with lzma.open(R/'build/dsky-two-integer-reference.snapshots.xz','rt')as f:
 for line in f:
  row=list(map(int,line.split()));number=row[0]//8;nmission=int(number>=4096)
  if number!=lastn:previous=inputs[number];lastn=number
  stage=row[0]%8
  candidates[stage][nmission].append((number,stage,previous[:2050],row[1:]))
  previous=row[1:]
selected=[]
for stage in range(5):
 rows=[]
 for mission in range(2):
  c=candidates[stage][mission];rows.extend(c[i*(len(c)-1)//15]for i in range(16))
 selected.append(rows)
def emit(path,rows):
 nums=[];st=[];bo=[0];ao=[0];bp=[];ap=[]
 for n,stage,before,after in rows:
  base=inputs[n];nums.append(n);st.append(stage)
  bp.extend((i,v)for i,(v,b)in enumerate(zip(before,base))if v!=b);bo.append(len(bp))
  ap.extend((i,v)for i,(v,b)in enumerate(zip(after,before))if v!=b);ao.append(len(ap))
 lines=['/* SPDX-License-Identifier: MIT */',f'#define DSKY_TWO_INTEGER_FROZEN_COUNT {len(rows)}u','static const uint16_t dsky_two_integer_numbers[]={'+','.join(map(str,nums))+'};','static const uint8_t dsky_two_integer_stages[]={'+','.join(map(str,st))+'};']
 for name,offsets,patch in [('before',bo,bp),('after',ao,ap)]:
  lines+=['static const uint16_t dsky_two_integer_'+name+'_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t dsky_two_integer_'+name+'_patch[][2]={']+[' {'+str(i)+','+str(v)+'},'for i,v in (patch if patch else[(0,0)])]+['};']
 (R/path).write_text('\n'.join(lines)+'\n')
 print(path,len(rows),'states',len(bp),'before/',len(ap),'after patches')
for stage,rows in enumerate(selected):emit('tests/dsky_two_integer_vectors_'+str(stage)+'.h',rows)
emit('tests/dsky_two_integer_vectors_all.h',[row for rows in selected for row in rows])
lines=['/* SPDX-License-Identifier: MIT */','#ifndef DSKY_TWO_INTEGER_VECTORS_H','#define DSKY_TWO_INTEGER_VECTORS_H','#ifdef DSKY_TWO_INTEGER_ONLY_STAGE']
for stage in range(5):lines+=[('#if'if stage==0 else'#elif')+' DSKY_TWO_INTEGER_ONLY_STAGE=='+str(stage),'#include "dsky_two_integer_vectors_'+str(stage)+'.h"']
lines+=['#endif','#else','#include "dsky_two_integer_vectors_all.h"','#endif','#endif',''];(R/'tests/dsky_two_integer_vectors.h').write_text('\n'.join(lines))
