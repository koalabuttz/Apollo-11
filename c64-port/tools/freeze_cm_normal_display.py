#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma
R=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-ffunction-sections','-Wl,--gc-sections','-Iinclude','-Itests','src/agc_word.c','tests/cm_trig_cases.c','tests/cm_clockjob_cases.c','tests/cm_normal_display_cases.c','tests/cm_nv85_cases.c','tools/cm_normal_display_inputs.c','-o','build/cm-normal-display-inputs'],cwd=R,check=True)
inputs=[list(map(int,x.split()))[2:] for x in subprocess.check_output(['build/cm-normal-display-inputs'],cwd=R,text=True).splitlines()]
selected=[n*30+p for n in [0,9,16,32,63,511,512,639] for p in ([7,8,10,11,12,16,20,28] if n<512 else [7,8,10,11,12,16,20,24])]
before_patch=[];after_patch=[];before_offsets=[0];after_offsets=[0];prior=None
with lzma.open(R/'build/cm-normal-display-reference.snapshots.xz','rt')as f:
 for line in f:
  index=int(line.split()[0])
  after=list(map(int,line.split()))[2:]
  if index in selected:
   n=index//30;before=inputs[n]if index%30==0 else prior
   before_patch.extend((i,v)for i,(v,b)in enumerate(zip(before,inputs[n]))if v!=b)
   after_patch.extend((i,v)for i,(v,b)in enumerate(zip(after,before))if v!=b)
   before_offsets.append(len(before_patch));after_offsets.append(len(after_patch))
  prior=after
assert len(before_offsets)==65
lines=['/* SPDX-License-Identifier: MIT */','#ifndef CM_NORMAL_DISPLAY_VECTORS_H','#define CM_NORMAL_DISPLAY_VECTORS_H','static const uint16_t cm_normal_display_numbers[]={'+','.join(map(str,selected))+'};']
for name,values in [('before_offsets',before_offsets),('after_offsets',after_offsets)]:lines+=['static const uint16_t cm_normal_display_'+name+'[]={'+','.join(map(str,values))+'};']
for name,values in [('before_patch',before_patch),('after_patch',after_patch)]:lines+=['static const uint16_t cm_normal_display_'+name+'[][2]={']+[' {'+str(i)+','+str(v)+'},'for i,v in values]+['};']
lines+=['#endif',''];(R/'tests/cm_normal_display_vectors.h').write_text('\n'.join(lines));print('Frozen64 complete original caller states;',len(before_patch),'input patches;',len(after_patch),'output patches')
