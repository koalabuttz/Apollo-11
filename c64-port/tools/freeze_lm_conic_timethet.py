#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze 8 oracle TIMETHET entries as lossless entry-image deltas for the
host and C64 target check (tests/lm_conic_timethet_vectors.h). Needs the stream
from tools/lm_conic_timethet_differential.py."""
import hashlib,json,lzma,pathlib,struct,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(R/'tools'))
from frozen_words import encode_rows,c_header
# Eight entries, small enough for the host target check beside the whole
# routine: together they take every phase transition TIMETHET makes on the
# 65536 sampled entries (GEOM's COLINEAR fallback, COGAOVFL and INFINITY into
# ABTCONIC, the GETX series and iteration loops, INVRSEQN and TRUE360X, both
# DELTAIL ends, the NEWSTATE update and the bare return), Earth and Moon
# entries, raw edge words (112), the longest chain (13002, 26 phases) and the
# first and last case. The host test also compares all 65536.
ids=[0,99,112,264,2360,3312,13002,65535]
subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','tools/lm_conic_timethet_inputs.c','tests/lm_conic_timethet_cases.c','tests/lm_conic_timethet_debris.c','-lm','-o','build/lm-conic-timethet-inputs'],cwd=R,check=True)
before=subprocess.check_output(['build/lm-conic-timethet-inputs',*map(str,ids)],cwd=R);assert len(before)==4100*len(ids)
want=set(ids);found={}
with lzma.open(R/'build/lm-conic-timethet-reference.snapshots.bin.xz','rb')as f:
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
(R/'tests/lm_conic_timethet_vectors.h').write_text('/* SPDX-License-Identifier: MIT; exact independently executed original words (tools/freeze_lm_conic_timethet.py). */\n#ifndef LM_CONIC_TIMETHET_VECTORS_H\n#define LM_CONIC_TIMETHET_VECTORS_H\n'+f'#define LM_CONIC_TIMETHET_MAX_CHANGES {maximum}u\n'+c_header(packed,'lm_conic_timethet')+'#endif\n')
result=dict(states=len(rows),max_changes=maximum,frozen_bytes=packed.storage_bytes,decoded_sha256=hashlib.sha256(source).hexdigest(),case_ids=ids)
(R/'evidence/lm-conic-timethet/lm-conic-timethet-target-states.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS',len(rows),'lossless original target states',packed.storage_bytes,'bytes, max changes',maximum)
