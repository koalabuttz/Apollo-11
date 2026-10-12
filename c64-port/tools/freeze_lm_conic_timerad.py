#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze 12 oracle TIMERAD/APSIDES entries as lossless entry-image deltas for the
host and C64 target check (tests/lm_conic_timerad_vectors.h). Needs the stream
from tools/lm_conic_timerad_differential.py."""
import hashlib,json,lzma,pathlib,struct,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(R/'tools'))
from frozen_words import encode_rows,c_header
# Twelve entries, small enough for the host target check beside the whole
# routine: together they take every phase transition TIMERAD and APSIDES make
# on the 65536 sampled entries (CIRCULAR, BADR2 from both tests, COGAOVFL,
# the GETX paths through INFINITY and TRUE360X, DELTIME, APSIDES' SQRT abort
# and INFINAPO from both branches), Earth and Moon entries, raw edge words,
# the longest chain and the first and last cases of each routine. The host
# test also compares all 65536.
ids=[0,1,2,19,114,120,122,368,379,4850,65534,65535]
subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','tools/lm_conic_timerad_inputs.c','tests/lm_conic_timerad_cases.c','tests/lm_conic_timerad_debris.c','-lm','-o','build/lm-conic-timerad-inputs'],cwd=R,check=True)
before=subprocess.check_output(['build/lm-conic-timerad-inputs',*map(str,ids)],cwd=R);assert len(before)==4100*len(ids)
want=set(ids);found={}
with lzma.open(R/'build/lm-conic-timerad-reference.snapshots.bin.xz','rb')as f:
 for n in range(65536):
  raw=f.read(4106);assert len(raw)==4106
  if n in want:found[n]=raw
 assert not f.read(1)
rows=[];maximum=0;source=bytearray()
for i,n in enumerate(ids):
 a=struct.unpack_from('<2050H',before,i*4100);b=struct.unpack('<2053H',found[n]);assert b[0]==n and b[1]==0;source.extend(found[n])
 delta=[(j,v)for j,(u,v)in enumerate(zip(a[:2048],b[5:]))if u!=v];maximum=max(maximum,len(delta))
 rows.append([n,b[2],b[3],b[4],len(delta),*sum(([j,v]for j,v in delta),[])])
for row in rows:row.extend([0]*(5+maximum*2-len(row)))
packed=encode_rows(rows)
(R/'tests/lm_conic_timerad_vectors.h').write_text('/* SPDX-License-Identifier: MIT; exact independently executed original words (tools/freeze_lm_conic_timerad.py). */\n#ifndef LM_CONIC_TIMERAD_VECTORS_H\n#define LM_CONIC_TIMERAD_VECTORS_H\n'+f'#define LM_CONIC_TIMERAD_MAX_CHANGES {maximum}u\n'+c_header(packed,'lm_conic_timerad')+'#endif\n')
result=dict(states=len(rows),max_changes=maximum,frozen_bytes=packed.storage_bytes,decoded_sha256=hashlib.sha256(source).hexdigest(),case_ids=ids)
(R/'evidence/lm-conic-timerad/lm-conic-timerad-target-states.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS',len(rows),'lossless original target states',packed.storage_bytes,'bytes, max changes',maximum)
