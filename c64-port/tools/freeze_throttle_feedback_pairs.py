#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
from pathlib import Path
import struct,sys,hashlib
root=Path(__file__).resolve().parents[1];sys.path.insert(0,str(root/'tools'))
from frozen_words import encode_rows,c_header
r=(root/'build/throttle-feedback-pairs-reference.snapshots').read_bytes();assert r==(root/'build/throttle-feedback-pairs-native.snapshots').read_bytes()
rows=[struct.unpack_from('<140H',r,o)for o in range(0,len(r),280)];groups=[[]for _ in range(20)];seen=[set()for _ in range(20)]
# Prefer corners and spread cases. Each frozen entry is from the assembly boundary.
wanted=set(range(16))|{4096+i*127 for i in range(8)}|{8192+i*1023 for i in range(8)}
for before,after in zip(rows,rows[1:]):
 step=before[-1]
 if before[0]!=after[0] or step>=20:continue
 if before[0]in wanted and before[0] not in seen[step]:groups[step].append(list(before[3:])+list(after[2:]));seen[step].add(before[0])
for before,after in zip(rows,rows[1:]):
 step=before[-1]
 if before[0]!=after[0] or step>=20 or len(groups[step])>=32 or before[0]in seen[step]:continue
 groups[step].append(list(before[3:])+list(after[2:]));seen[step].add(before[0])
s='/* SPDX-License-Identifier: MIT\n * SHA256:'+hashlib.sha256(r).hexdigest()+' */\n#ifndef FEEDBACK_PAIRS_VECTORS_H\n#define FEEDBACK_PAIRS_VECTORS_H\n'
for step,group in enumerate(groups):
 group=group[:32];assert len(group)==32
 s+=f'#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED=={step}\n'+c_header(encode_rows(group),f'feedback_pair{step}')+'#endif\n'
s+='#endif\n';(root/'tests/throttle_feedback_pairs_vectors.h').write_text(s)
