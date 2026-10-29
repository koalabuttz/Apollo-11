#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma
R=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-Iinclude','-Itests','src/agc_word.c','tests/dsky_decimal_load_cases.c','tools/dsky_decimal_load_inputs.c','-o','build/dsky-decimal-load-inputs'],cwd=R,check=True)
selected=[group*4096+n for group in range(2)for n in list(range(32))+list(range(64,96))]
inputs={int(row.split()[0]):list(map(int,row.split()))[1:]for row in subprocess.check_output(['build/dsky-decimal-load-inputs'],cwd=R,text=True).splitlines()if int(row.split()[0])in selected}
ao=[0];ap=[]
with lzma.open(R/'build/dsky-decimal-load-reference.snapshots.xz','rt')as f:
 for line in f:
  row=list(map(int,line.split()));n=row[0]
  if n in selected:
   ap.extend((i,v)for i,(v,b)in enumerate(zip(row[1:],inputs[n]))if v!=b)
   ao.append(len(ap))
assert len(ao)==129
lines=['/* SPDX-License-Identifier: MIT */','#ifndef DSKY_DECIMAL_LOAD_VECTORS_H','#define DSKY_DECIMAL_LOAD_VECTORS_H','static const uint16_t dsky_decimal_load_numbers[]={'+','.join(map(str,selected))+'};','static const uint16_t dsky_decimal_load_after_offsets[]={'+','.join(map(str,ao))+'};','static const uint16_t dsky_decimal_load_after_patch[][2]={']+[' {'+str(i)+','+str(v)+'},'for i,v in ap]+['};','#endif',''];(R/'tests/dsky_decimal_load_vectors.h').write_text('\n'.join(lines));print('Frozen128 full original DECDSP fresh data load states;',len(ap),'source output patches')
