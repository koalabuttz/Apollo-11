#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import hashlib,json,lzma,pathlib,struct,subprocess
from frozen_words import encode_rows,c_header
R=pathlib.Path(__file__).resolve().parents[1]
def main():
 local=list(range(16))+[31,32,63,64,127,128,255,256,1023,8191,8192,16383,16384,32766,32767,65535]
 ids=[mission*131072+operation*65536+n for mission in range(2) for operation in range(2) for n in local]
 before=subprocess.check_output([str(R/'build/sband-trig-inputs'),*map(str,ids)],cwd=R);assert len(before)==4100*len(ids)
 originals={};records=bytearray();rows=[];maximum=0
 for mission,name in [(0,'LM'),(1,'CM')]:
  with lzma.open(R/'build'/('sband-trig-'+name+'-native.snapshots.bin.xz'),'rb') as f:
   n=0
   while raw:=f.read(4106):
    assert len(raw)==4106
    if mission*131072+n in ids:originals[mission*131072+n]=raw
    n+=1
  assert n==131072
 for i,n in enumerate(ids):
  raw=originals[n];a=struct.unpack_from('<2050H',before,i*4100);b=struct.unpack('<2053H',raw)
  delta=[(j,v)for j,(u,v)in enumerate(zip(a[:2048],b[5:]))if u!=v];maximum=max(maximum,len(delta));rows.append([n&65535,n>>16,b[2],b[3],b[4],len(delta),*sum(([j,v]for j,v in delta),[])]);records.extend(raw)
 for row in rows:row.extend([0]*(6+maximum*2-len(row)))
 packed=encode_rows(rows);(R/'tests/sband_trig_vectors.h').write_text('/* SPDX-License-Identifier: MIT; exact independently executed original words. */\n#ifndef SBAND_TRIG_VECTORS_H\n#define SBAND_TRIG_VECTORS_H\n'+f'#define SBAND_TRIG_MAX_CHANGES {maximum}u\n'+c_header(packed,'sband_trig')+'#endif\n')
 with lzma.open(R/'build/sband-trig-target-source.snapshots.bin.xz','wb',preset=6) as f:f.write(records)
 result=dict(states=len(ids),max_changes=maximum,frozen_bytes=packed.storage_bytes,decoded_sha256=hashlib.sha256(records).hexdigest(),case_ids=ids);(R/'build/sband-trig-target-results.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS',result['states'],'exact full-image frozen states',packed.storage_bytes,'fixture bytes')
if __name__=='__main__':main()
