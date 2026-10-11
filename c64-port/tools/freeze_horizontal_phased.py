#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
from pathlib import Path
import struct,sys,hashlib
r=Path(__file__).resolve().parents[1];sys.path.insert(0,str(r/'tools'));from frozen_words import encode_rows,c_header
raw=(r/'build/horizontal-phased-reference.snapshots').read_bytes();assert raw==(r/'build/horizontal-phased-native.snapshots').read_bytes()
a=[struct.unpack_from('<135H',raw,o)for o in range(0,len(raw),270)];groups=[[]for _ in range(13)];seen=[set()for _ in range(13)]
want=set(range(16))|{4096+i*255 for i in range(16)}
for preferred in [True,False]:
 for before,after in zip(a,a[1:]):
  if before[0]!=after[0]:continue
  prev=before[1];step=0 if prev==13 else 4 if prev==3 else prev+1
  if step>=13 or len(groups[step])>=32 or before[0]in seen[step]or(preferred and before[0]not in want):continue
  groups[step].append([step,before[2]]+list(before[3:])+list(after[1:]));seen[step].add(before[0])
s='/* SPDX-License-Identifier: MIT\n * SHA256:'+hashlib.sha256(raw).hexdigest()+' */\n#ifndef HORIZONTAL_PHASED_VECTORS_H\n#define HORIZONTAL_PHASED_VECTORS_H\n'
for step,group in enumerate(groups):
 assert len(group)==32
 s+=f'#if !defined(HORIZONTAL_PHASE_SELECTED) || HORIZONTAL_PHASE_SELECTED=={step}\n'+c_header(encode_rows(group),f'horizontal_phase{step}')+'#endif\n'
s+='#endif\n';(r/'tests/horizontal_phased_vectors.h').write_text(s)
