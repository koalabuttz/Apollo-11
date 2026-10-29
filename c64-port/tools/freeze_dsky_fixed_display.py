#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma
R=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-Iinclude','-Itests','tests/dsky_fixed_display_cases.c','tools/dsky_fixed_display_inputs.c','-o','build/dsky-fixed-display-inputs'],cwd=R,check=True)
base=[0,1024,2048,3072,23552+1023,24576,27648+1023,28672+1,31744+1023,32768,35840+1023,36864,39936+1023,257,34816+511,39936]
numbers=base+[40960+n for n in base];inputs={int(row.split()[0]):list(map(int,row.split()))[1:]+[99]for row in subprocess.check_output(['build/dsky-fixed-display-inputs']+list(map(str,numbers)),cwd=R,text=True).splitlines()};outputs={}
with lzma.open(R/'build/dsky-fixed-display-reference.snapshots.xz','rt')as f:
 for line in f:
  key=int(line.split(' ',1)[0]);n,stage=divmod(key,3)
  if n in inputs:outputs[key]=list(map(int,line.split()))[1:]
assert len(outputs)==96
def emit(path,stages):
 cases=[];pre=[];post=[];ip=[];op=[];ioff=[0];ooff=[0];values=[];steps=[]
 for stage in stages:
  for n in numbers:
   a=inputs[n]if stage==0 else outputs[n*3+stage-1][:-1]+[99];b=outputs[n*3+stage];cases.append(n);steps.append(stage);values.append(outputs[n*3+1][0]&0o77777)
   ip.extend((i,v)for i,(v,base)in enumerate(zip(a,inputs[n]))if v!=base);ioff.append(len(ip));op.extend((i,v)for i,(v,base)in enumerate(zip(b,a))if v!=base);ooff.append(len(op))
 lines=['/* SPDX-License-Identifier: MIT */',f'#define DSKY_FIXED_DISPLAY_FROZEN_COUNT {len(cases)}u','static const uint32_t dsky_fixed_display_numbers[]={'+','.join(str(n)+'UL'for n in cases)+'};','static const uint8_t dsky_fixed_display_stages[]={'+','.join(map(str,steps))+'};','static const agc_word dsky_fixed_display_values[]={'+','.join(map(str,values))+'};']
 for name,arr in [('input_offsets',ioff),('output_offsets',ooff)]:lines+=['static const uint16_t dsky_fixed_display_'+name+'[]={'+','.join(map(str,arr))+'};']
 for name,arr in [('input_patch',ip),('output_patch',op)]:lines+=['static const uint16_t dsky_fixed_display_'+name+'[][2]={']+[' {'+str(i)+','+str(v)+'},'for i,v in (arr or [(0,0)])]+['};']
 (R/path).write_text('\n'.join(lines)+'\n');print(path,len(cases),'complete states',len(ip),len(op),'patches')
for stage in range(3):emit('tests/dsky_fixed_display_vectors_'+str(stage)+'.h',[stage])
emit('tests/dsky_fixed_display_vectors_all.h',range(3))
(R/'tests/dsky_fixed_display_vectors.h').write_text('''/* SPDX-License-Identifier: MIT */
#ifndef DSKY_FIXED_DISPLAY_VECTORS_H
#define DSKY_FIXED_DISPLAY_VECTORS_H
#ifdef DSKY_FIXED_DISPLAY_ONLY_STAGE
#if DSKY_FIXED_DISPLAY_ONLY_STAGE==0
#include "dsky_fixed_display_vectors_0.h"
#elif DSKY_FIXED_DISPLAY_ONLY_STAGE==1
#include "dsky_fixed_display_vectors_1.h"
#else
#include "dsky_fixed_display_vectors_2.h"
#endif
#else
#include "dsky_fixed_display_vectors_all.h"
#endif
#endif
''')
