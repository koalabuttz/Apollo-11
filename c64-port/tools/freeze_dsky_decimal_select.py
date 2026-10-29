#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma
R=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-Iinclude','-Itests','src/agc_word.c','tests/dsky_decimal_select_cases.c','tools/dsky_decimal_select_inputs.c','-o','build/dsky-decimal-select-inputs'],cwd=R,check=True)
selected=[group*4096+n for group in range(2)for n in ([6*r+k for r in range(14)for k in range(3)]+[6*r+3+r%3 for r in range(14)]+[(28 if group==0 else 19)*84+q for q in range(8)])]
selected.sort()
inputs={int(row.split()[0]):list(map(int,row.split()))[1:]for row in subprocess.check_output(['build/dsky-decimal-select-inputs'],cwd=R,text=True).splitlines()if int(row.split()[0])in selected}
ao=[0];ap=[]
with lzma.open(R/'build/dsky-decimal-select-reference.snapshots.xz','rt')as f:
 for line in f:
  row=list(map(int,line.split()));n=row[0]
  if n in selected:
   ap.extend((i,v)for i,(v,b)in enumerate(zip(row[1:],inputs[n]))if v!=b)
   ao.append(len(ap))
assert len(ao)==129
lines=['/* SPDX-License-Identifier: MIT */','#ifndef DSKY_DECIMAL_SELECT_VECTORS_H','#define DSKY_DECIMAL_SELECT_VECTORS_H','static const uint16_t dsky_decimal_select_numbers[]={'+','.join(map(str,selected))+'};','static const uint16_t dsky_decimal_select_after_offsets[]={'+','.join(map(str,ao))+'};','static const uint16_t dsky_decimal_select_after_patch[][2]={']+[' {'+str(i)+','+str(v)+'},'for i,v in ap]+['};','#endif',''];(R/'tests/dsky_decimal_select_vectors.h').write_text('\n'.join(lines));print('Frozen128 full original DSPDCPUT factor/scale selection states;',len(ap),'source output patches')
