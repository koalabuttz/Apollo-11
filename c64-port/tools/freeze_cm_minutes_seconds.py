#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma
R=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-Iinclude','-Itests','src/agc_word.c','tests/cm_decimal_pair_cases.c','tests/cm_minutes_seconds_cases.c','tools/cm_minutes_seconds_inputs.c','-o','build/cm-minutes-seconds-inputs'],cwd=R,check=True)
inputs=[list(map(int,x.split()))[2:]for x in subprocess.check_output(['build/cm-minutes-seconds-inputs'],cwd=R,text=True).splitlines()]
selected=list(range(64))+[1024+n*6+p for n in [0,8,9,17,25,32,41,511]for p in range(6)]
bp=[];ap=[];bo=[0];ao=[0]
with lzma.open(R/'build/cm-decimal-pair-reference.snapshots.xz','rt')as f:
 for index,line in enumerate(f):
  if index not in selected:continue
  after=list(map(int,line.split()))[2:];before=inputs[index]
  ap.extend((i,v)for i,(v,b)in enumerate(zip(after,before))if v!=b);bo.append(len(bp));ao.append(len(ap))
prior=None
with lzma.open(R/'build/cm-minutes-seconds-reference.snapshots.xz','rt')as f:
 for index,line in enumerate(f):
  after=list(map(int,line.split()))[2:]
  if 1024+index in selected:
   n=index//6;before=inputs[1024+n]if index%6==0 else prior
   bp.extend((i,v)for i,(v,b)in enumerate(zip(before,inputs[1024+n]))if v!=b)
   ap.extend((i,v)for i,(v,b)in enumerate(zip(after,before))if v!=b);bo.append(len(bp));ao.append(len(ap))
  prior=after
assert len(bo)==113
lines=['/* SPDX-License-Identifier: MIT */','#ifndef CM_MINUTES_SECONDS_VECTORS_H','#define CM_MINUTES_SECONDS_VECTORS_H','static const uint16_t cm_ms_numbers[]={'+','.join(map(str,selected))+'};']
for name,values in [('before_offsets',bo),('after_offsets',ao)]:lines+=['static const uint16_t cm_ms_'+name+'[]={'+','.join(map(str,values))+'};']
for name,values in [('before_patch',bp),('after_patch',ap)]:lines+=['static const uint16_t cm_ms_'+name+'[][2]={']+[' {'+str(i)+','+str(v)+'},'for i,v in values]+['};']
lines+=['#endif',''];(R/'tests/cm_minutes_seconds_vectors.h').write_text('\n'.join(lines));print('Frozen112 complete original states;',len(bp),'input patches;',len(ap),'output patches')
