#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze complete independent original LUNPOS polynomial and return states as lossless image deltas."""
import hashlib,json,lzma,pathlib,struct,subprocess
from frozen_words import encode_rows,c_header
R=pathlib.Path(__file__).resolve().parents[1]
ids=list(range(32))+[128+i*2051 for i in range(32)]
subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','tools/cm_ephemeris_solar_inputs.c','tests/cm_ephemeris_time_cases.c','-o','build/cm-ephemeris-solar-inputs'],cwd=R,check=True)
rows=[];maximum=0;source=bytearray()
for mission,name in enumerate(['CM']):
 before=subprocess.check_output(['build/cm-ephemeris-solar-inputs',name,*map(str,ids)],cwd=R);assert len(before)==4100*len(ids)
 originals={}
 with lzma.open(R/'build'/'cm-ephemeris-solar-reference.snapshots.bin.xz','rb')as f:
  for n in range(65536):
   raw=f.read(4106);assert len(raw)==4106
   if n in ids:originals[n]=raw
  assert not f.read(1)
 for i,n in enumerate(ids):
  raw=originals[n];a=struct.unpack_from('<2050H',before,i*4100);b=struct.unpack('<2053H',raw);source.extend(raw)
  delta=[(j,v)for j,(u,v)in enumerate(zip(a[:2048],b[5:]))if u!=v];maximum=max(maximum,len(delta));rows.append([n,mission,b[2],b[3],b[4],len(delta),*sum(([j,v]for j,v in delta),[])])
for row in rows:row.extend([0]*(6+maximum*2-len(row)))
packed=encode_rows(rows);(R/'tests/cm_ephemeris_solar_vectors.h').write_text('/* SPDX-License-Identifier: MIT; exact independently executed original words. */\n#ifndef CM_EPHEMERIS_SOLAR_VECTORS_H\n#define CM_EPHEMERIS_SOLAR_VECTORS_H\n'+f'#define CM_EPHEMERIS_SOLAR_MAX_CHANGES {maximum}u\n'+c_header(packed,'cm_ephemeris_solar')+'#endif\n')
with lzma.open(R/'build/cm-ephemeris-solar-target-source.snapshots.bin.xz','wb',preset=6)as f:f.write(source)
result=dict(states=len(rows),max_changes=maximum,frozen_bytes=packed.storage_bytes,decoded_sha256=hashlib.sha256(source).hexdigest(),case_ids=ids)
(R/'build/cm-ephemeris-solar-target-results.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS',len(rows),'lossless original target states',packed.storage_bytes,'bytes')
