#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Lossless complete-source-image fixtures; no native output is used."""
import pathlib,struct,hashlib,subprocess,gzip,lzma
from frozen_words import encode_rows,c_header
r=pathlib.Path(__file__).resolve().parents[1]
indices=[0,1,2,3,8191,8192,8193,16383,16384,16385,24575,24576,24577,32765,32766,32767]
subprocess.run(['gcc','-O2','-std=c99','-Iinclude','-Itests','tools/cm_powered_trig_inputs.c','tests/cm_powered_trig_cases.c','src/agc_word.c','-o','build/cm-powered-trig-inputs'],cwd=r,check=True)
heads=b''.join(struct.pack('<2H',n,live)for live in range(2)for n in indices)
data=subprocess.check_output(['build/cm-powered-trig-inputs'],input=heads,cwd=r);initial={};k=0
for live in range(2):
 for n in indices:initial[live,n]=struct.unpack_from('<2049H',data,k);k+=4098
source={};digest=hashlib.sha256();path=r/'build/cm-powered-trig-reference.snapshots.gz'
if not path.exists():path=r/'evidence/cm-powered-trig-complete.snapshots.xz'
with (lzma.open if path.suffix=='.xz' else gzip.open)(path,'rb')as f:
 while raw:=f.read(4104):
  assert len(raw)==4104;digest.update(raw);x=struct.unpack('<2052H',raw);n,live,phase,irq=x[:4]
  if n in indices:source[phase,live,n]=(x[4:],irq)
s='/* SPDX-License-Identifier: MIT\n * Original complete-source corpus SHA256:'+digest.hexdigest()+'\n * Constructor SHA256:'+hashlib.sha256((r/'tests/cm_powered_trig_cases.c').read_bytes()).hexdigest()+' */\n#ifndef CM_POWERED_TRIG_VECTORS_H\n#define CM_POWERED_TRIG_VECTORS_H\n'
for phase in range(10):
 rows=[]
 for live in range(2):
  for n in indices:
   after,irq=source[phase,live,n];changes=[(i,v)for i,v in enumerate(after)if v!=initial[live,n][i+1]]
   rows.append([n,live,irq,len(changes)]+[v for pair in changes for v in pair])
 width=max(map(len,rows));packed=encode_rows([row+[0]*(width-len(row))for row in rows])
 s+=f'#if !defined(CM_POWERED_TRIG_SELECTED) || CM_POWERED_TRIG_SELECTED=={phase}\n'+c_header(packed,f'cm_powered_trig{phase}')+'#endif\n';print(phase,packed.storage_bytes)
s+='#endif\n';(r/'tests/cm_powered_trig_vectors.h').write_text(s)
