#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import hashlib,json,lzma,pathlib,struct,subprocess
from frozen_words import encode_rows,c_header
R=pathlib.Path(__file__).resolve().parents[1]
def main():
 ids=[g*65536+n for g in range(2) for n in (0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 47, 48, 63, 64, 95, 96, 127, 128, 255, 256, 511, 512, 1023, 1024, 2047, 2048, 4095, 4096, 8191, 8192, 16383, 16384, 24575, 24576, 32767, 32768, 49151, 49152, 65534, 65535)]
 before=subprocess.check_output([str(R/'build/sband-exit-inputs'),*map(str,ids)],cwd=R);assert len(before)==4100*len(ids)
 originals={};records=bytearray();rows=[];maximum=0
 with lzma.open(R/'build/sband-exit-native.snapshots.bin.xz','rb') as f:
  while raw:=f.read(4106):
   assert len(raw)==4106;nlo,nhi=struct.unpack_from('<2H',raw);n=nlo|nhi<<16
   if n in ids:originals[n]=raw
 for i,n in enumerate(ids):
  raw=originals[n];a=struct.unpack_from('<2050H',before,i*4100);b=struct.unpack('<2053H',raw);delta=[(j,v)for j,(u,v)in enumerate(zip(a[:2048],b[5:]))if u!=v];maximum=max(maximum,len(delta));rows.append([n&65535,n>>16,b[2],b[3],b[4],len(delta),*sum(([j,v]for j,v in delta),[])]);records.extend(raw)
 for row in rows:row.extend([0]*(6+maximum*2-len(row)))
 packed=encode_rows(rows);(R/'tests/sband_exit_vectors.h').write_text('/* SPDX-License-Identifier: MIT; independently executed original source. */\n#ifndef SBAND_EXIT_VECTORS_H\n#define SBAND_EXIT_VECTORS_H\n'+f'#define SBAND_EXIT_MAX_CHANGES {maximum}u\n'+c_header(packed,'sband_exit')+'#endif\n')
 with lzma.open(R/'build/sband-exit-target-source.snapshots.bin.xz','wb',preset=6)as f:f.write(records)
 result=dict(states=len(ids),max_changes=maximum,frozen_bytes=packed.storage_bytes,decoded_sha256=hashlib.sha256(records).hexdigest(),case_ids=ids);(R/'build/sband-exit-target-results.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS',len(ids),'frozen states;',packed.storage_bytes,'fixture bytes')
if __name__=='__main__':main()
