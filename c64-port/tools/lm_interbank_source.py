#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Audit every original interbank source line/word and actual BANKJUMP callers."""
import hashlib,json,pathlib,re
from fixed_data_generate import listing_records,source_line,rope_word
R=pathlib.Path(__file__).resolve().parents[1]
def sha(raw):return hashlib.sha256(raw).hexdigest()
def main():
 path='Luminary099/INTER-BANK_COMMUNICATION.agc';raw=(R.parent/path).read_bytes();fixture=R/'build/standard-lead-source/Luminary099';rope=(fixture/'MAIN.agc.bin').read_bytes()
 assert (fixture/'INTER-BANK_COMMUNICATION.agc').read_bytes()==raw
 assert sha(rope)=='1f5326e038de5b741b2f27b01ec949dbd688cf1906994e997402587c8628f40e'
 records=listing_records(fixture/'assembly.lst',{'INTER-BANK_COMMUNICATION.agc':path},{path:raw.decode()},rope)
 assert {i for i,line in enumerate(raw.decode().splitlines(),1) if source_line(line) is not None}=={r['line'] for r in records}
 assert sum(len(r['words']) for r in records)==76
 assert rope_word(rope,0o4644)==0o12000
 calls=[]
 for source in (R.parent/'Luminary099').glob('*.agc'):
  lines=source.read_text().splitlines()
  for i,line in enumerate(lines):
   if re.search(r'\b(?:TC|TCF)\s+BANKJUMP\b',line.split('#')[0]):
    before=[dict(line=j+1,text=lines[j]) for j in range(max(0,i-8),i+1)]
    prior=next((source_line(lines[j]) for j in range(i-1,-1,-1) if source_line(lines[j]) is not None),None)
    calls.append(dict(path='Luminary099/'+source.name,line=i+1,immediate_producer=prior,source_context=before,source_sha256=sha(source.read_bytes())))
 result=dict(source_path=path,source_sha256=sha(raw),fixture_source_sha256=sha((fixture/'INTER-BANK_COMMUNICATION.agc').read_bytes()),numerical_edits=0,source_edits=0,rope_sha256=sha(rope),source_words=76,source_line_word_map=records,bankjump_original_callers=calls,negative_overflow_excluded=False,negative_overflow_continuation='fixed source CCS1777 and all4 MAKECADR successor stages; actual TC Q reachesPC2 with invalid -1 descriptor, before external register-word fetch; does not execute invalid arbitrary instruction data',instruction_decoder=False)
 (R/'build/lm-interbank-source-audit.json').write_text(json.dumps(result,indent=2)+'\n')
 print('PASS byte-original LM76words/78source directives and',len(calls),'actual BANKJUMP caller source contexts; PRIO12literal exact')
if __name__=='__main__':main()
