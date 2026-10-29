#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Original antenna vector store/mixing and pending UNIT/PDVL caller map."""
import hashlib,json,pathlib
from fixed_data_generate import listing_records,symbols,fixed_address,rope_word,physical_erasable
R=pathlib.Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 result={};audits=json.loads((R/'build/sband-source/audit.json').read_text())
 for mission,name in [('LM','Luminary099'),('CM','Comanche055')]:
  work=R/'build/sband-source'/name;sy=symbols((work/'assembly.lst').read_text());rope=(work/'MAIN.agc.bin').read_bytes();audit=next(x for x in audits if x['source_path'].startswith(name+'/'))
  lo,hi=(0o3654,0o3671)if mission=='LM'else(0o3712,0o3714)
  selected=[x for x in audit['source_line_word_map']if x['words']and lo<=int(x['assembled_address'].split(',')[-1],8)<hi];constants={}
  if mission=='LM':
   at=fixed_address(sy['1OVSQRT2']);words=[rope_word(rope,at+i)for i in range(2)];assert words==[0o26501,0o7463]
   original=R.parent/name/'S-BAND_ANTENNA_FOR_LM.agc';records=listing_records(work/'assembly.lst',{'S-BAND_ANTENNA_FOR_LM.agc':audit['source_path']},{audit['source_path']:original.read_text()},rope);data=[x for x in records if x['words']and x['physical_fixed_word']is not None and at<=x['physical_fixed_word']<at+2];assert sum((x['words']for x in data),[])==words
   constants=dict(label='1OVSQRT2',address=sy['1OVSQRT2'],physical_fixed_word=at,words=words,source_line_word_map=data,usage='Both original DP words consumed by the two true DMP kernels; no padded instruction data.')
   assert physical_erasable(sy['RLM'])==0o2204
  result[mission]=dict(source_path=audit['source_path'],source_sha256=audit['source_sha256'],rope_sha256=audit['rope_sha256'],source_edits=0,containing_file_status='partial',native_source='src/sband_navframe.c',source_words=sum(len(x['words'])for x in selected),source_line_word_map=selected,constants=constants,bindings={k:sy[k]for k in ['NEWOPS','DANZIG','UNIT','SQRTABRT','PDVL']},scope='Actual returnedSMNB vector STORE; LM full STODL/PUSH/DSU/DMP/pop/DAD/DMP/STOVL body mixing; complete unchanged normal UNIT prologue and three arithmetic phases. Stops pendingPDVL or beforeSQRTABRT; PDVL operand and later antenna bodies not executed.',alias_contract='LM originalY pushed then popped; STODL keeps old resultminorL and final VLOAD keeps oldZminorL. CMSTORE exchanges oldtargetZminor intoL. Single canonical2048word image; no authoritative module snapshots.',caller_controls='Actual original NEWOPS LOC3654LM/3712CM, sourceBANKSET/FB064000 plus SUPERbit100; coherentEBBB, MODE-1, fiveVAC/PUSHbaseLM/base+010CM, NEWJOB+0 and producer exclusion.',coverage='65536inputs/mission, complete raw16A/L and ordinaryQ marginals, fiveVAC/eightEB/four bank42SUPERforms/bothIRQ, varied ordinary/signedzero vectors. No Cartesian or every instruction edge claim.')
 (R/'build/sband-navframe-source-map.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS nav-frame source maps',[(m,x['source_words'])for m,x in result.items()])
if __name__=='__main__':main()
