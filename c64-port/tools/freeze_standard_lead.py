#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Exact original full image target fixtures; sparse storage, all words checked."""
from pathlib import Path
from frozen_words import encode_rows,c_header
R=Path(__file__).resolve().parents[1]
def selected(path,ids):
 result={}
 with path.open() as f:
  for n,line in enumerate(f):
   if n in ids:result[n]=list(map(int,line.split()))
 assert set(result)==ids
 return result
def main():
 for mission,short in enumerate(('lm','cm')):
  groups=[('stage',[(p*512+k,0) for p in range(7) for k in (0,8,32,504)],1),('bank',[(n,p) for n in (0,1,7,8,15,16,31,52,63,127,255,511,767,1023,1535,2047) for p in range(3)],3),('interp',[(n,p) for n in (0,1,31,47,63,127,191,255,319,383,511,639,767,895,1007,1023) for p in range(4)],4),('wake',[(n,0) for n in (0,1,7,8,15,63,127,128,255,511,767,1023,1279,1535,1791,2047)],1)]
  if mission:groups.append(('extra',[(p*512+k,0) for p in range(3) for k in range(8)],1))
  groups=[(kind,*group) for kind,group in enumerate(groups)]
  groups.append((5,'resume',[(n,0) for n in (0,1,255,511,512,513,767,1023)],1))
  rows=[]
  for kind,label,cases,steps in groups:
   before=selected(R/f'build/standard-lead-input-{mission}-{kind}.snapshots',{n for n,p in cases})
   after=selected(R/f'build/standard-lead-{short}-{label}-reference.snapshots',{n*steps+p for n,p in cases})
   for n,phase in cases:
    source=after[n*steps+phase];initial=before[n];changes=[(i,b) for i,(a,b) in enumerate(zip(initial[2:],source[2:])) if a!=b];assert len(source)==len(initial)==2050
    superbank=0o160 if kind!=4 else n%512%4*0o20
    irq=n//512 if kind==5 else 1 if kind==2 else ((n%512%2) if kind==4 and n<1024 else int(kind==4 and n%8!=7))
    rows.append([kind,n,phase,source[1],superbank,irq,len(changes)]+[v for pair in changes for v in pair])
  maximum=max(row[6] for row in rows)
  for row in rows:row.extend([0]*(2*(maximum-row[6])))
  packed=encode_rows(rows);text='/* SPDX-License-Identifier: MIT; exact independently executed original source states. */\n#ifndef STANDARD_LEAD_VECTORS_H\n#define STANDARD_LEAD_VECTORS_H\n'+c_header(packed,'standard_lead')+f'#define STANDARD_LEAD_TARGET_MAX_CHANGES {maximum}u\n#endif\n'
  (R/f'tests/standard_lead_vectors_{short}.h').write_text(text)
  print(short,len(rows),'target whole images;',maximum,'maximum changed words;',packed.storage_bytes,'lossless fixture bytes')
if __name__=='__main__':main()
