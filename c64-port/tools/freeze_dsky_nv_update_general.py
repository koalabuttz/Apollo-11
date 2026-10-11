#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma
R=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-Iinclude','-Itests','src/agc_word.c','tests/dsky_nv_update_general_cases.c','tools/dsky_nv_update_general_inputs.c','-o','build/dsky-nv-update-general-inputs'],cwd=R,check=True)
selected=[group*1024+n for group in range(8)for n in [0,1,2,3,16,17,18,19,100,201,302,403,808,909,1010,1023]]
inputs={int(row.split()[0]):list(map(int,row.split()))[1:]for row in subprocess.check_output(['build/dsky-nv-update-general-inputs'],cwd=R,text=True).splitlines()if int(row.split()[0])in selected}
ao=[0];ap=[];routes=[]
with lzma.open(R/'build/dsky-nv-update-general-reference.snapshots.xz','rt')as f:
 for line in f:
  row=list(map(int,line.split()));n=row[0]
  if n in selected:
   ap.extend((i,v)for i,(v,b)in enumerate(zip(row[2:],inputs[n]))if v!=b)
   ao.append(len(ap));routes.append(row[1])
assert len(ao)==129
lines=['/* SPDX-License-Identifier: MIT */','#ifndef DSKY_NV_UPDATE_GENERAL_VECTORS_H','#define DSKY_NV_UPDATE_GENERAL_VECTORS_H','static const uint16_t dsky_nv_update_general_numbers[]={'+','.join(map(str,selected))+'};','static const uint8_t dsky_nv_update_general_routes[]={'+','.join(map(str,routes))+'};','static const uint16_t dsky_nv_update_general_after_offsets[]={ '+','.join(map(str,ao))+'};','static const uint16_t dsky_nv_update_general_after_patch[][2]={']+[' {'+str(i)+','+str(v)+'},'for i,v in ap]+['};','#endif',''];(R/'tests/dsky_nv_update_general_vectors.h').write_text('\n'.join(lines));print('Frozen128 full original UPDATVB/UPDATNN states;',len(ap),'source output patches')
