#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma
R=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-ffunction-sections','-Wl,--gc-sections','-Iinclude','-Itests','src/agc_word.c','tests/cm_trig_cases.c','tests/cm_clockjob_cases.c','tests/cm_nv85_cases.c','tools/cm_nv85_inputs.c','-o','build/cm-nv85-inputs'],cwd=R,check=True)
inputs=[list(map(int,x.split()))[2:] for x in subprocess.check_output(['build/cm-nv85-inputs'],cwd=R,text=True).splitlines()]
control=[7,8,9,10,11,12,14,15,16,18,19,20,22,23,24]
selected=[n*26+p for n in [0,1,127,511] for p in control]
before_patch=[];after_patch=[];before_offsets=[0];after_offsets=[0];prior=None
with lzma.open(R/'build/cm-nv85-reference.snapshots.xz','rt')as f:
 for index,line in enumerate(f):
  after=list(map(int,line.split()))[2:]
  if index in selected:
   n=index//26;before=inputs[n]if index%26==0 else prior
   before_patch.extend((i,v)for i,(v,b)in enumerate(zip(before,inputs[n]))if v!=b)
   after_patch.extend((i,v)for i,(v,b)in enumerate(zip(after,before))if v!=b)
   before_offsets.append(len(before_patch));after_offsets.append(len(after_patch))
  prior=after
assert len(before_offsets)==61
lines=['/* SPDX-License-Identifier: MIT */','#ifndef CM_NV85_VECTORS_H','#define CM_NV85_VECTORS_H','static const uint16_t cm_nv85_numbers[]={'+','.join(map(str,selected))+'};']
for name,values in [('before_offsets',before_offsets),('after_offsets',after_offsets)]:lines+=['static const uint16_t cm_nv85_'+name+'[]={'+','.join(map(str,values))+'};']
for name,values in [('before_patch',before_patch),('after_patch',after_patch)]:lines+=['static const uint16_t cm_nv85_'+name+'[][2]={']+[' {'+str(i)+','+str(v)+'},'for i,v in values]+['};']
lines+=['#endif',''];(R/'tests/cm_nv85_vectors.h').write_text('\n'.join(lines));print('Frozen60 complete original caller states;',len(before_patch),'input patches;',len(after_patch),'output patches')
