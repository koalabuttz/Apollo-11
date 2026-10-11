#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Lossless full source image fixtures from source-only independent outputs."""
import pathlib,struct,hashlib,subprocess,gzip
from frozen_words import encode_rows,c_header
r=pathlib.Path(__file__).resolve().parents[1]
indices=list(range(8))+[8191,8192,16383,16384,32767,32768,49151,49152,65535,65536,65537,65551,65552,65791,65792,69631,69632,73727]
# Include the independently observed first case of every source basic path.
import re
for pc,n in re.findall(r'sourcePC([0-7]+) completed_instruction_visits\d+ firstcase(\d+)',(r/'build/cm-mysubs-reference.trace' if (r/'build/cm-mysubs-reference.trace').exists() else r/'evidence/cm-mysubs-reference.trace').read_text()):
 if int(n)<73728 and int(n)not in indices:indices.append(int(n))
assert len(indices)<=32,indices
for n in [65553,65807,69633,65538,65539,65540,65541]:
 if len(indices)==32:break
 if n not in indices:indices.append(n)
assert len(indices)==32,indices
subprocess.run(['gcc','-O2','-std=c99','-Iinclude','-Itests','tools/cm_mysubs_inputs.c','tests/cm_mysubs_cases.c','src/agc_word.c','-o','build/cm-mysubs-inputs'],cwd=r,check=True)
heads=b''.join(struct.pack('<3H',n&65535,n>>16,op%2)for op in range(4)for n in indices);data=subprocess.check_output(['build/cm-mysubs-inputs'],input=heads,cwd=r);initial={};k=0
for op in range(4):
 for n in indices:initial[op,n]=struct.unpack_from('<2049H',data,k);k+=4098
source={};digest=hashlib.sha256()
source_path=r/'build/cm-mysubs-reference.snapshots.gz'
with (gzip.open(source_path if source_path.exists() else r/'evidence/cm-mysubs-complete.snapshots.gz','rb')) as f:
 while raw:=f.read(4106):
  digest.update(raw);x=struct.unpack('<2053H',raw);nlo,nhi,selector,phase,irq=x[:5];n=nlo+(nhi<<16);op=selector+2*phase
  if n in indices:source[op,n]=(x[5:],irq)
s='/* SPDX-License-Identifier: MIT\n * Complete2048-word source-image delta fixtures. Original corpusSHA256:'+digest.hexdigest()+'\n * ConstructorSHA256:'+hashlib.sha256((r/'tests/cm_mysubs_cases.c').read_bytes()).hexdigest()+' */\n#ifndef CM_MYSUBS_VECTORS_H\n#define CM_MYSUBS_VECTORS_H\n'
for op in range(4):
 rows=[]
 for n in indices:
  after,irq=source[op,n];changes=[(i,v)for i,v in enumerate(after)if v!=initial[op,n][i+1]];rows.append([n&65535,n>>16,irq,len(changes)]+[v for pair in changes for v in pair])
 width=max(map(len,rows));packed=encode_rows([row+[0]*(width-len(row))for row in rows]);s+=f'#if !defined(CM_MYSUBS_SELECTED) || CM_MYSUBS_SELECTED=={op}\n'+c_header(packed,f'cm_mysubs{op}')+'#endif\n';print(op,packed.storage_bytes)
s+='#endif\n';(r/'tests/cm_mysubs_vectors.h').write_text(s)
