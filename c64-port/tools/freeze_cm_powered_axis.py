#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Lossless complete-source-image fixtures; no native output is used."""
import pathlib,struct,hashlib,subprocess,gzip,lzma
from frozen_words import encode_rows,c_header
r=pathlib.Path(__file__).resolve().parents[1]
indices=[0,1,2,3,4,5,6,7,8190,8191,8192,8193,16382,16383,16384,16385,24574,24575,24576,24577,32766,32767,32768,32769,49150,49151,49152,49153,65532,65533,65534,65535]
subprocess.run(['gcc','-O2','-std=c99','-Iinclude','-Itests','tools/cm_powered_axis_inputs.c','tests/cm_powered_axis_cases.c','tests/cm_powered_trig_cases.c','src/agc_word.c','-o','build/cm-powered-axis-inputs'],cwd=r,check=True)
heads=b''.join(struct.pack('<2H',n&65535,n>>16)for n in indices)
data=subprocess.check_output(['build/cm-powered-axis-inputs'],input=heads,cwd=r);initial={};k=0
for n in indices:initial[n]=struct.unpack_from('<2049H',data,k);k+=4098
source={};digest=hashlib.sha256();path=r/'build/cm-powered-axis-reference.snapshots.gz'
if not path.exists():path=r/'evidence/cm-powered-axis-complete.snapshots.xz'
with (lzma.open if path.suffix=='.xz' else gzip.open)(path,'rb')as f:
 while raw:=f.read(4104):
  assert len(raw)==4104;digest.update(raw);x=struct.unpack('<2052H',raw);nlo,nhi,phase,irq=x[:4];n=nlo+(nhi<<16)
  if n in indices:source[phase,n]=(x[4:],irq)
s='/* SPDX-License-Identifier: MIT\n * Original complete-source corpus SHA256:'+digest.hexdigest()+'\n * Constructor SHA256:'+hashlib.sha256((r/'tests/cm_powered_axis_cases.c').read_bytes()).hexdigest()+' */\n#ifndef CM_POWERED_AXIS_VECTORS_H\n#define CM_POWERED_AXIS_VECTORS_H\n'
for phase in range(2):
 rows=[]
 for n in indices:
  after,irq=source[phase,n];changes=[(i,v)for i,v in enumerate(after)if v!=initial[n][i+1]]
  rows.append([n&65535,n>>16,irq,len(changes)]+[v for pair in changes for v in pair])
 width=max(map(len,rows));packed=encode_rows([row+[0]*(width-len(row))for row in rows])
 s+=f'#if !defined(CM_POWERED_AXIS_SELECTED) || CM_POWERED_AXIS_SELECTED=={phase}\n'+c_header(packed,f'cm_powered_axis{phase}')+'#endif\n';print(phase,packed.storage_bytes)
s+='#endif\n';(r/'tests/cm_powered_axis_vectors.h').write_text(s)
