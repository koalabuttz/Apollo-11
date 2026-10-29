#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source provenance and emitted-word mapping for both SIGN/COMP groups."""
import pathlib,re,json,hashlib
r=pathlib.Path(__file__).resolve().parents[1];result={}
for mission,name,audit,lo,hi in [(0,'Luminary099','lm-powered-source-audit.json',1423,1453),(1,'Comanche055','cm-powered-trig-source-audit.json',1431,1461)]:
 short='lm'if mission==0 else'cm';source=r.parent/name/'INTERPRETER.agc';fixture=r/f'deps/{short}-powered-fixture';provenance=json.loads((r/'evidence'/audit).read_text());record=provenance['original_sources']['INTERPRETER.agc'];assert hashlib.sha256(source.read_bytes()).hexdigest()==record['original_sha256'];assembled_sha=hashlib.sha256((fixture/'INTERPRETER.agc').read_bytes()).hexdigest();assert assembled_sha==record.get('assembled_source_sha256',record.get('normalized_sha256'));assert hashlib.sha256((fixture/'MAIN.agc.bin').read_bytes()).hexdigest()==provenance['rope_sha256']
 listing=(fixture/'assembly.lst').read_text().split('$INTERPRETER.agc',1)[1].split('\n',1)[1];words=[]
 for line in listing.splitlines():
  if re.match(r'^\d+,\d+: .*\$[^ ]+\.agc',line):break
  m=re.match(r'^\d+,(\d+):\s+([0-7]{4})\s+([0-7]{5})',line)
  if not m or not lo<=int(m[1])<=hi:continue
  address=int(m[2],8);assert 0o6000<=address<0o10000
  words.append({'line':int(m[1]),'bank':'03','address':m[2],'physical_fixed_word':address,'raw':m[3]})

 assert len(words)==27,len(words)
 result[name]={'original_source_sha256':record['original_sha256'],'assembled_source_sha256':assembled_sha,'support_rope_sha256':provenance['rope_sha256'],'source_audit_dependency':'evidence/'+audit,'source_edits':record.get('source_edits',0),'indentation_repairs':record.get('indentation_repair_lines',[]),'sign_source_lines':[lo,hi],'emitted_words':len(words),'words':words}
(r/'build/interpreted-sign-source-audit.json').write_text(json.dumps(result,indent=2)+'\n')
print('Original SIGN/COMP source words',[v['emitted_words']for v in result.values()])
