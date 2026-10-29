#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma
R=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-Iinclude','-Itests','src/agc_word.c','tests/cm_display_rest_cases.c','tools/cm_display_rest_inputs.c','-o','build/cm-display-rest-inputs'],cwd=R,check=True)
selected=[group*1024+n for group in range(3)for n in [0,1,2,3,4,7,8,15,16,31,32,63,64,127,128,255,256,511,767,1023]]
inputs={int(row.split()[0]):list(map(int,row.split()))[1:]for row in subprocess.check_output(['build/cm-display-rest-inputs'],cwd=R,text=True).splitlines()if int(row.split()[0])in selected}
ao=[0];ap=[]
with lzma.open(R/'build/cm-display-rest-reference.snapshots.xz','rt')as f:
 for line in f:
  row=list(map(int,line.split()));n=row[0]
  if n in selected:
   ap.extend((i,v)for i,(v,b)in enumerate(zip(row[1:],inputs[n]))if v!=b)
   ao.append(len(ap))
assert len(ao)==61
lines=['/* SPDX-License-Identifier: MIT */','#ifndef CM_DISPLAY_REST_VECTORS_H','#define CM_DISPLAY_REST_VECTORS_H','static const uint16_t cm_display_rest_numbers[]={'+','.join(map(str,selected))+'};','static const uint16_t cm_display_rest_after_offsets[]={'+','.join(map(str,ao))+'};','static const uint16_t cm_display_rest_after_patch[][2]={']+[' {'+str(i)+','+str(v)+'},'for i,v in ap]+['};','#endif',''];(R/'tests/cm_display_rest_vectors.h').write_text('\n'.join(lines));print('Frozen60 full original NVDSP/REST/RESTSLEP states;',len(ap),'source output patches')
