#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma
R=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-Iinclude','-Itests','src/agc_word.c','tests/dsky_decimal_time_consumer_cases.c','tools/dsky_decimal_time_consumer_inputs.c','-o','build/dsky-decimal-time-consumer-inputs'],cwd=R,check=True)
inputs={int(row.split()[0]):list(map(int,row.split()))[1:]for row in subprocess.check_output(['build/dsky-decimal-time-consumer-inputs'],cwd=R,text=True).splitlines()}
candidates=[[[],[]]for _ in range(22)]
previous=None;lastn=-1
with lzma.open(R/'build/dsky-decimal-time-consumer-reference.snapshots.xz','rt')as f:
 for line in f:
  row=list(map(int,line.split()));number=row[0]//128;nmission=int(number>=2048)
  if number!=lastn:previous=inputs[number]+[3];lastn=number
  pc=previous[5]
  pcs=[[0o3230,0o3234,0o3240,0o3242,0o3253,0o3255,0o3264,0o3266,0o3303,0o3343,0o3347,0o3351,0o3361,0o3371,0o2732,0o2733,0o2734,0o2736,0o2745],[0o3143,0o3147,0o3153,0o3155,0o3166,0o3170,0o3177,0o3201,0o3216,0o3256,0o3262,0o3264,0o3274,0o3304,0o2636,0o2637,0o2640,0o2642,0o2651]]
  stage=0 if pc==0o2523 else 1 if pc==0o2537 else 21 if pc==0o2570 else pcs[nmission].index(pc)+2
  candidates[stage][nmission].append((number,stage,previous[:2050]+[3],row[1:]))
  previous=row[1:]
selected=[]
for stage in range(22):
 rows=[]
 for mission in range(2):
  c=candidates[stage][mission];rows.extend(c[i*(len(c)-1)//15]for i in range(16))
 selected.append(rows)
def emit(path,rows):
 nums=[];st=[];bo=[0];ao=[0];bp=[];ap=[]
 for n,stage,before,after in rows:
  base=inputs[n]+[3];nums.append(n);st.append(stage)
  bp.extend((i,v)for i,(v,b)in enumerate(zip(before,base))if v!=b);bo.append(len(bp))
  ap.extend((i,v)for i,(v,b)in enumerate(zip(after,before))if v!=b);ao.append(len(ap))
 lines=['/* SPDX-License-Identifier: MIT */',f'#define DSKY_DECIMAL_TIME_CONSUMER_FROZEN_COUNT {len(rows)}u','static const uint16_t dsky_decimal_time_consumer_numbers[]={'+','.join(map(str,nums))+'};','static const uint8_t dsky_decimal_time_consumer_stages[]={'+','.join(map(str,st))+'};']
 for name,offsets,patch in [('before',bo,bp),('after',ao,ap)]:
  lines+=['static const uint16_t dsky_decimal_time_consumer_'+name+'_offsets[]={'+','.join(map(str,offsets))+'};','static const uint16_t dsky_decimal_time_consumer_'+name+'_patch[][2]={']+[' {'+str(i)+','+str(v)+'},'for i,v in (patch if patch else[(0,0)])]+['};']
 (R/path).write_text('\n'.join(lines)+'\n')
 print(path,len(rows),'states',len(bp),'before/',len(ap),'after patches')
for stage,rows in enumerate(selected):emit('tests/dsky_decimal_time_consumer_vectors_'+str(stage)+'.h',rows)
emit('tests/dsky_decimal_time_consumer_vectors_all.h',[row for rows in selected for row in rows])
lines=['/* SPDX-License-Identifier: MIT */','#ifndef DSKY_DECIMAL_TIME_CONSUMER_VECTORS_H','#define DSKY_DECIMAL_TIME_CONSUMER_VECTORS_H','#ifdef DSKY_DECIMAL_TIME_CONSUMER_ONLY_STAGE']
for stage in range(22):lines+=[('#if'if stage==0 else'#elif')+' DSKY_DECIMAL_TIME_CONSUMER_ONLY_STAGE=='+str(stage),'#include "dsky_decimal_time_consumer_vectors_'+str(stage)+'.h"']
lines+=['#endif','#else','#include "dsky_decimal_time_consumer_vectors_all.h"','#endif','#endif',''];(R/'tests/dsky_decimal_time_consumer_vectors.h').write_text('\n'.join(lines))
