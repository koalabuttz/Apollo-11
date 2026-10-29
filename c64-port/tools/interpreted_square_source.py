#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source provenance and emitted-word mapping for both DSQ/DSQSUB groups."""
import pathlib,re,json,hashlib
r=pathlib.Path(__file__).resolve().parents[1];result={}
for mission,name,audit,lo,hi in [(0,'Luminary099','lm-powered-source-audit.json',2338,2463),(1,'Comanche055','cm-powered-trig-source-audit.json',2346,2471)]:
 short='lm'if mission==0 else'cm';source=r.parent/name/'INTERPRETER.agc';fixture=r/f'deps/{short}-powered-fixture';provenance=json.loads((r/'evidence'/audit).read_text());record=provenance['original_sources']['INTERPRETER.agc'];assert hashlib.sha256(source.read_bytes()).hexdigest()==record['original_sha256'];assembled_sha=hashlib.sha256((fixture/'INTERPRETER.agc').read_bytes()).hexdigest();assert assembled_sha==record.get('assembled_source_sha256',record.get('normalized_sha256'));assert hashlib.sha256((fixture/'MAIN.agc.bin').read_bytes()).hexdigest()==provenance['rope_sha256']
 listing=(fixture/'assembly.lst').read_text().split('$INTERPRETER.agc',1)[1].split('\n',1)[1];words=[]
 for line in listing.splitlines():
  if re.match(r'^\d+,\d+: .*\$[^ ]+\.agc',line):break
  m=re.match(r'^\d+,(\d+):\s+([0-7]{2}),([0-7]{4})\s+([0-7]{5})(?:\s+([0-7]{5}))?',line)
  if not m or not (lo<=int(m[1])<=lo+1 or hi-14<=int(m[1])<=hi):continue
  for index,raw in enumerate([m[4],m[5]]):
   if raw is None:continue
   address=int(m[3],8)+index;physical=int(m[2],8)*1024+address-1024
   words.append({'line':int(m[1]),'bank':m[2],'address':f'{address:04o}','physical_fixed_word':physical,'raw':raw})

 assert len(words)==17,len(words)
 result[name]={'original_source_sha256':record['original_sha256'],'assembled_source_sha256':assembled_sha,'support_rope_sha256':provenance['rope_sha256'],'source_audit_dependency':'evidence/'+audit,'source_edits':record.get('source_edits',0),'indentation_repairs':record.get('indentation_repair_lines',[]),'DSQ_source_lines':[lo,lo+1],'DSQSUB_source_lines':[hi-14,hi],'emitted_words':len(words),'words':words}
(r/'build/interpreted-square-source-audit.json').write_text(json.dumps(result,indent=2)+'\n')
print('Original DSQ/DSQSUB source words',[v['emitted_words']for v in result.values()])
