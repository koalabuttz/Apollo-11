#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma
R=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-Iinclude','-Itests','src/agc_word.c','tests/matrix_cases.c','tests/cm_dyndisp_cases.c','tools/cm_dyndisp_inputs.c','-o','build/cm-dyndisp-inputs'],cwd=R,check=True)
inputs=[list(map(int,x.split()))[2:] for x in subprocess.check_output(['build/cm-dyndisp-inputs'],cwd=R,text=True).splitlines()]
control=[0,1,2,3,4,5,6,7,9,10,15,17,22,23,24,25,26]
selected=[n*27+p for n in [0,1,127,511] for p in control]
before_patch=[];after_patch=[];before_offsets=[0];after_offsets=[0];prior=None
with lzma.open(R/'build/cm-dyndisp-reference.snapshots.xz','rt')as f:
 for index,line in enumerate(f):
  after=list(map(int,line.split()))[2:]
  if index in selected:
   n=index//27;before=inputs[n]if index%27==0 else prior
   before_patch.extend((i,v)for i,(v,b)in enumerate(zip(before,inputs[n]))if v!=b)
   after_patch.extend((i,v)for i,(v,b)in enumerate(zip(after,before))if v!=b)
   before_offsets.append(len(before_patch));after_offsets.append(len(after_patch))
  prior=after
assert len(before_offsets)==69
lines=['/* SPDX-License-Identifier: MIT */','#ifndef CM_DYNDISP_VECTORS_H','#define CM_DYNDISP_VECTORS_H','static const uint16_t cm_dyndisp_numbers[]={'+','.join(map(str,selected))+'};']
for name,values in [('before_offsets',before_offsets),('after_offsets',after_offsets)]:lines+=['static const uint16_t cm_dyndisp_'+name+'[]={'+','.join(map(str,values))+'};']
for name,values in [('before_patch',before_patch),('after_patch',after_patch)]:lines+=['static const uint16_t cm_dyndisp_'+name+'[][2]={']+[' {'+str(i)+','+str(v)+'},'for i,v in values]+['};']
lines+=['#endif',''];(R/'tests/cm_dyndisp_vectors.h').write_text('\n'.join(lines));print('Frozen68 complete original caller states;',len(before_patch),'input patches;',len(after_patch),'output patches')
