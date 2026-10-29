#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Full-state source-only fixtures for original CM RCS matrix update."""
import pathlib,struct,hashlib,subprocess,gzip,lzma,json
from frozen_words import encode_rows,c_header
r=pathlib.Path(__file__).resolve().parents[1]
indices=list(range(8))+[659,660,661,8191,8192,8193,16383,16384,16385,32767,32768,32769,49151,49152,49153,65535,65536,65537,65538,65539,65540,65541,65542,65543,73726,73727,73728,73729,73730,73731,73732,73733,81918,81919,81920,81921,81922,81923,81924,81925,98302,98303,98304,98305,98306,98307,114686,114687,114688,114689,114690,114691,131068,131069,131070,131071]
assert len(indices)==64
subprocess.run(['gcc','-O2','-std=c99','-Iinclude','-Itests','tools/cm_rcs_matrix_inputs.c','tests/cm_rcs_matrix_cases.c','src/agc_word.c','-o','build/cm-rcs-matrix-inputs'],cwd=r,check=True)
data=subprocess.check_output(['build/cm-rcs-matrix-inputs'],input=b''.join(struct.pack('<I',n)for n in indices),cwd=r);initial={n:struct.unpack_from('<2049H',data,k*4098)for k,n in enumerate(indices)};source={};h=hashlib.sha256();count=0;gate=[0,0,0];path=r/'build/cm-rcs-matrix-reference.snapshots.gz'
if not path.exists():path=r/'evidence/cm-rcs-matrix-complete.snapshots.xz'
with(lzma.open if path.suffix=='.xz'else gzip.open)(path,'rb')as f:
 while raw:=f.read(4102):
  assert len(raw)==4102;h.update(raw);w=struct.unpack('<2051H',raw);n=w[0]+(w[1]<<16);irq=w[2];after=w[3:];count+=1
  flag=(0o20000|((n>>2)&0o17777))if n<65536 else(n&0o77777)
  gate[0 if flag==0 or flag&0o40000 else(1 if not flag&0o20000 else 2)]+=1
  if n in initial:source[n]=(after,irq)
rows=[]
for n in indices:
 after,irq=source[n];changes=[(i,v)for i,v in enumerate(after)if v!=initial[n][i+1]];rows.append([n&65535,n>>16,irq,len(changes)]+[v for pair in changes for v in pair])
width=max(map(len,rows));packed=encode_rows([row+[0]*(width-len(row))for row in rows]);s='/* SPDX-License-Identifier: MIT\n * Complete original corpus SHA256:'+h.hexdigest()+' */\n#ifndef CM_RCS_MATRIX_VECTORS_H\n#define CM_RCS_MATRIX_VECTORS_H\n'+c_header(packed,'cm_rcs_matrix')+'#endif\n';(r/'tests/cm_rcs_matrix_vectors.h').write_text(s);x={'complete_states':count,'decoded_sha256':h.hexdigest(),'branch_counts':{'negative_or_zero':gate[0],'non_RCS':gate[1],'RCS_update':gate[2]},'frozen_states':64,'frozen_storage_bytes':packed.storage_bytes};(r/'build/cm-rcs-matrix-corpus-stats.json').write_text(json.dumps(x,indent=2)+'\n');print(x)
