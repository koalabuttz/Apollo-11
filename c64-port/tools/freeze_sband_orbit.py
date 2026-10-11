#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import hashlib,json,lzma,pathlib,struct,subprocess
from frozen_words import encode_rows,c_header
R=pathlib.Path(__file__).resolve().parents[1]
def main():
 ids=[g*65536+n for g in range(2) for n in (*range(16),31,32,63,64,127,128,255,256,257,263,264,1023,32766,32767,32768,65535)]
 before=subprocess.check_output([str(R/'build/sband-orbit-inputs'),*map(str,ids)],cwd=R);assert len(before)==4100*len(ids)
 originals={};records=bytearray();rows=[];maximum=0
 with lzma.open(R/'build/sband-orbit-native.snapshots.bin.xz','rb') as f:
  while raw:=f.read(4106):
   assert len(raw)==4106;nlo,nhi=struct.unpack_from('<2H',raw);n=nlo|nhi<<16
   if n in ids:originals[n]=raw
 for i,n in enumerate(ids):
  raw=originals[n];a=struct.unpack_from('<2050H',before,i*4100);b=struct.unpack('<2053H',raw);delta=[(j,v)for j,(u,v)in enumerate(zip(a[:2048],b[5:]))if u!=v];maximum=max(maximum,len(delta));rows.append([n&65535,n>>16,b[2],b[3],b[4],len(delta),*sum(([j,v]for j,v in delta),[])]);records.extend(raw)
 for row in rows:row.extend([0]*(6+maximum*2-len(row)))
 packed=encode_rows(rows);(R/'tests/sband_orbit_vectors.h').write_text('/* SPDX-License-Identifier: MIT; independently executed original source. */\n#ifndef SBAND_ORBIT_VECTORS_H\n#define SBAND_ORBIT_VECTORS_H\n'+f'#define SBAND_ORBIT_MAX_CHANGES {maximum}u\n'+c_header(packed,'sband_orbit')+'#endif\n')
 with lzma.open(R/'build/sband-orbit-target-source.snapshots.bin.xz','wb',preset=6)as f:f.write(records)
 result=dict(states=len(ids),max_changes=maximum,frozen_bytes=packed.storage_bytes,decoded_sha256=hashlib.sha256(records).hexdigest(),case_ids=ids);(R/'build/sband-orbit-target-results.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS',len(ids),'frozen states;',packed.storage_bytes,'fixture bytes')
if __name__=='__main__':main()
