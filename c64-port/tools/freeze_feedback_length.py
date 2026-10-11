#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
from pathlib import Path
import struct,sys,hashlib
r=Path(__file__).resolve().parents[1];sys.path.insert(0,str(r/'tools'));from frozen_words import encode_rows,c_header
raw=(r/'build/feedback-length-reference.snapshots').read_bytes();assert raw==(r/'build/feedback-length-native.snapshots').read_bytes();a=[struct.unpack_from('<140H',raw,o)for o in range(0,len(raw),280)];groups=[[]for _ in range(4)];want=set(range(16))|{4096+i*127 for i in range(8)}|{8192+i*1023 for i in range(8)}
for before,after in zip(a,a[1:]):
 if before[0]!=after[0] or before[0]not in want:continue
 step=after[1]-210
 if 0<=step<4:groups[step].append(list(before[3:])+list(after[2:]))
s='/* SPDX-License-Identifier: MIT\n * SHA256:'+hashlib.sha256(raw).hexdigest()+' */\n#ifndef FEEDBACK_LENGTH_VECTORS_H\n#define FEEDBACK_LENGTH_VECTORS_H\n'
for step,g in enumerate(groups):
 assert len(g)==32;s+=f'#if !defined(FEEDBACK_LENGTH_SELECTED) || FEEDBACK_LENGTH_SELECTED=={step}\n'+c_header(encode_rows(g),f'feedback_length{step}')+'#endif\n'
s+='#endif\n';(r/'tests/feedback_length_vectors.h').write_text(s)
