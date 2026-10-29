#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze sparse exact source states, retaining all untouched canonical words."""
import hashlib,json,pathlib,struct,subprocess,lzma
from frozen_words import encode_rows,c_header
R=pathlib.Path(__file__).resolve().parents[1]
def main():
 edges=[0,32,65535,65503,16384,16416,32767,32735,32768,32800,49151,49119,49152,49184,1,65534]
 ids=[k*65536+n for k in range(14) for n in edges]
 ids += [917504+k*256+n for k in range(14) for n in (0,4,8,12,16,20,32,36,48,52,64,68,128,132,240,244)]
 baseline=subprocess.check_output([str(R/'build/lm-interbank-input-dump'),*map(str,ids)],cwd=R)
 assert len(baseline)==4108*len(ids)
 rows=[];records=bytearray();maximum=0
 for index,n in enumerate(ids):
  source=subprocess.check_output([str(R/'build/lm-interbank-reference-v3'),str(R/'build/standard-lead-source/Luminary099/MAIN.agc.bin'),str(n),str(n+1)],cwd=R)
  native=subprocess.check_output([str(R/'build/lm-interbank-native-v3'),str(n),str(n+1)],cwd=R)
  assert source==native and len(source)==4108,(n,'source/native')
  before=struct.unpack_from('<2054H',baseline,index*4108);after=struct.unpack('<2054H',source)
  delta=[(i,b) for i,(a,b) in enumerate(zip(before[4:2052],after[4:2052])) if a!=b]
  maximum=max(maximum,len(delta));word=after[4+0o135] if after[2]==6 else 0
  rows.append([n&65535,n>>16,after[2],before[3],word,after[3],after[2052],after[2053],len(delta),*sum(([i,v] for i,v in delta),[])])
  records.extend(source)
 for row in rows:row.extend([0]*(9+2*maximum-len(row)))
 packed=encode_rows(rows)
 path=R/'tests/lm_interbank_vectors.h'
 path.write_text('/* SPDX-License-Identifier: MIT; independently executed original LM source states. */\n#ifndef LM_INTERBANK_VECTORS_H\n#define LM_INTERBANK_VECTORS_H\n'+f'#define LM_INTERBANK_MAX_CHANGES {maximum}u\n'+c_header(packed,'lm_interbank')+'#endif\n')
 raw=R/'build/lm-interbank-target-source.snapshots.bin.xz'
 with lzma.open(raw,'wb',preset=6) as f:f.write(records)
 result=dict(states=len(ids),word_fields=2048,external_fields=['superbank','interrupt_enabled'],max_changes=maximum,frozen_bytes=packed.storage_bytes,decoded_sha256=hashlib.sha256(records).hexdigest(),case_ids=ids)
 (R/'build/lm-interbank-target-results.json').write_text(json.dumps(result,indent=2)+'\n')
 print('PASS',len(ids),'independent source states; sparse reconstruction validates every2048word;',packed.storage_bytes,'fixture bytes')
if __name__=='__main__':main()
