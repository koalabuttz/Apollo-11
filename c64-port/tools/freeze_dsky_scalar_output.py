#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma
R=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-Iinclude','-Itests','src/agc_word.c','tests/dsky_scalar_output_cases.c','tools/dsky_scalar_output_inputs.c','-o','build/dsky-scalar-output-inputs'],cwd=R,check=True)
selected=[g*2048+n for g in range(10)for n in list(range(12))+[32,63,255,2047]]
selected.sort()
inputs={int(row.split()[0]):list(map(int,row.split()))[1:]for row in subprocess.check_output(['build/dsky-scalar-output-inputs'],cwd=R,text=True).splitlines()if int(row.split()[0])in selected}
ao=[0];ap=[]
with lzma.open(R/'build/dsky-scalar-output-reference.snapshots.xz','rt')as f:
 for line in f:
  row=list(map(int,line.split()));n=row[0]
  if n in selected:
   ap.extend((i,v)for i,(v,b)in enumerate(zip(row[1:],inputs[n]))if v!=b)
   ao.append(len(ap))
assert len(ao)==161
lines=['/* SPDX-License-Identifier: MIT */','#ifndef DSKY_SCALAR_OUTPUT_VECTORS_H','#define DSKY_SCALAR_OUTPUT_VECTORS_H','static const uint16_t dsky_scalar_output_numbers[]={'+','.join(map(str,selected))+'};','static const uint16_t dsky_scalar_output_after_offsets[]={'+','.join(map(str,ao))+'};','static const uint16_t dsky_scalar_output_after_patch[][2]={']+[' {'+str(i)+','+str(v)+'},'for i,v in ap]+['};','#endif',''];(R/'tests/dsky_scalar_output_vectors.h').write_text('\n'.join(lines));print('Frozen160 full original scalar output scale states;',len(ap),'source output patches')
