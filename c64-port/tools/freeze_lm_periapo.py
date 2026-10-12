#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze 20 oracle PERIAPO family entries as lossless entry-image deltas for the
host and C64 target check (tests/lm_periapo_vectors.h). Needs the stream
from tools/lm_periapo_differential.py."""
import hashlib,json,lzma,pathlib,struct,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(R/'tools'))
from frozen_words import encode_rows,c_header
# Twenty entries, small enough for the host target check beside the whole
# chain: together they take every phase transition the sampled entries make
# (PERIAPO1 with SETRAD on the Earth and Moon paths, APSIDES' SQRT abort and
# INFINAPO, PERIAPO, VECSHIFT, SHIFTR1, MAXCHK both ways, LEMSTORE and
# CSMSTORE both ways), raw edge words, SUPERBNK 0 and 3, and the last cases.
# The host test also compares all 65536.
ids=[0,1,3,4,5,6,7,8,11,12,15,22,23,63,320,323,326,4864,65534,65535]
subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','tools/lm_periapo_inputs.c','tests/lm_periapo_cases.c','tests/lm_periapo_debris.c','-lm','-o','build/lm-periapo-inputs'],cwd=R,check=True)
before=subprocess.check_output(['build/lm-periapo-inputs',*map(str,ids)],cwd=R);assert len(before)==4100*len(ids)
want=set(ids);found={}
with lzma.open(R/'build/lm-periapo-reference.snapshots.bin.xz','rb')as f:
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
(R/'tests/lm_periapo_vectors.h').write_text('/* SPDX-License-Identifier: MIT; exact independently executed original words (tools/freeze_lm_periapo.py). */\n#ifndef LM_PERIAPO_VECTORS_H\n#define LM_PERIAPO_VECTORS_H\n'+f'#define LM_PERIAPO_MAX_CHANGES {maximum}u\n'+c_header(packed,'lm_periapo')+'#endif\n')
result=dict(states=len(rows),max_changes=maximum,frozen_bytes=packed.storage_bytes,decoded_sha256=hashlib.sha256(source).hexdigest(),case_ids=ids)
(R/'evidence/lm-periapo/lm-periapo-target-states.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS',len(rows),'lossless original target states',packed.storage_bytes,'bytes, max changes',maximum)
