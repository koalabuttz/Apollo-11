#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma
R=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-Iinclude','-Itests','src/agc_word.c','tests/dsky_nv_return_cases.c','tools/dsky_nv_return_inputs.c','-o','build/dsky-nv-return-inputs'],cwd=R,check=True)
selected=[group*4096+n for group in range(2)for n in [0,1,2,3,7,8,15,31,32,63,64,127,128,255,256,511,512,767,1023,1024,1535,1791,2047,2048,2303,2559,2815,3071,3327,3583,3839,4095]]
inputs={int(row.split()[0]):list(map(int,row.split()))[1:]for row in subprocess.check_output(['build/dsky-nv-return-inputs'],cwd=R,text=True).splitlines()if int(row.split()[0])in selected}
ao=[0];ap=[]
with lzma.open(R/'build/dsky-nv-return-reference.snapshots.xz','rt')as f:
 for line in f:
  row=list(map(int,line.split()));n=row[0]
  if n in selected:
   ap.extend((i,v)for i,(v,b)in enumerate(zip(row[1:],inputs[n]))if v!=b)
   ao.append(len(ap))
assert len(ao)==65
lines=['/* SPDX-License-Identifier: MIT */','#ifndef DSKY_NV_RETURN_VECTORS_H','#define DSKY_NV_RETURN_VECTORS_H','static const uint16_t dsky_nv_return_numbers[]={'+','.join(map(str,selected))+'};','static const uint16_t dsky_nv_return_after_offsets[]={'+','.join(map(str,ao))+'};','static const uint16_t dsky_nv_return_after_patch[][2]={']+[' {'+str(i)+','+str(v)+'},'for i,v in ap]+['};','#endif',''];(R/'tests/dsky_nv_return_vectors.h').write_text('\n'.join(lines));print('Frozen64 full original NVSUBEND/SUPDXCHZ states;',len(ap),'source output patches')
