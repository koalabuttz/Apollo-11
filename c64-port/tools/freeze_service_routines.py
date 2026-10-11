#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze exact original source deltas including untouched input words."""
import hashlib,json,pathlib,struct,subprocess,lzma
from frozen_words import encode_rows,c_header
R=pathlib.Path(__file__).resolve().parents[1]
def main():
 ids=[k*4096+n for k in range(26) for n in (0,1,2,3,4,5,6,7,15,32,63,64,127,128,255,4095)]
 baseline=subprocess.check_output([str(R/'build/service-routines-input-dump'),*map(str,ids)],cwd=R);assert len(baseline)==4114*len(ids)
 rows=[];records=bytearray();maximum=0
 for index,n in enumerate(ids):
  source=subprocess.check_output([str(R/'build/service-routines-reference'),str(R/'build/service-routines-source/Luminary099/MAIN.agc.bin'),str(R/'build/service-routines-source/Comanche055/MAIN.agc.bin'),str(n),str(n+1)],cwd=R)
  native=subprocess.check_output([str(R/'build/service-routines-native'),str(n),str(n+1)],cwd=R);assert source==native and len(source)==4108,(n,'source/native')
  before=struct.unpack_from('<2057H',baseline,index*4114);after=struct.unpack('<2054H',source)
  delta=[(i,b) for i,(a,b) in enumerate(zip(before[4:2052],after[4:2052])) if a!=b];maximum=max(maximum,len(delta))
  rows.append([n&65535,n>>16,after[2],*before[2054:2057],after[3],after[2052],after[2053],len(delta),*sum(([i,v] for i,v in delta),[])]);records.extend(source)
 for row in rows:row.extend([0]*(10+2*maximum-len(row)))
 packed=encode_rows(rows)
 (R/'tests/service_routines_vectors.h').write_text('/* SPDX-License-Identifier: MIT; independently executed original source states. */\n#ifndef SERVICE_ROUTINES_VECTORS_H\n#define SERVICE_ROUTINES_VECTORS_H\n'+f'#define SERVICE_ROUTINES_MAX_CHANGES {maximum}u\n'+c_header(packed,'service_routines')+'#endif\n')
 with lzma.open(R/'build/service-routines-target-source.snapshots.bin.xz','wb',preset=6) as f:f.write(records)
 result=dict(states=len(ids),word_fields=2048,external_fields=['superbank','interrupt_enabled'],max_changes=maximum,frozen_bytes=packed.storage_bytes,decoded_sha256=hashlib.sha256(records).hexdigest(),case_ids=ids)
 (R/'build/service-routines-target-results.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS',len(ids),'full source states;',packed.storage_bytes,'lossless fixture bytes')
if __name__=='__main__':main()
