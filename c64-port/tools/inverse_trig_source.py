#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source provenance and emitted-word mapping for both inverse-trig groups."""
import pathlib,re,json,hashlib
r=pathlib.Path(__file__).resolve().parents[1];result={};rows=[]
for mission,name,audit,lo,hi in [(0,'Luminary099','lm-powered-source-audit.json',2705,2812),(1,'Comanche055','cm-powered-trig-source-audit.json',2708,2805)]:
 short='lm'if mission==0 else'cm';source=r.parent/name/'INTERPRETER.agc';fixture=r/f'deps/{short}-powered-fixture';provenance=json.loads((r/'evidence'/audit).read_text());record=provenance['original_sources']['INTERPRETER.agc'];assert hashlib.sha256(source.read_bytes()).hexdigest()==record['original_sha256'];assembled_sha=hashlib.sha256((fixture/'INTERPRETER.agc').read_bytes()).hexdigest();assert assembled_sha==record.get('assembled_source_sha256',record.get('normalized_sha256'));assert hashlib.sha256((fixture/'MAIN.agc.bin').read_bytes()).hexdigest()==provenance['rope_sha256']
 listing=(fixture/'assembly.lst').read_text().split('$INTERPRETER.agc',1)[1].split('\n',1)[1];words=[]
 for line in listing.splitlines():
  if re.match(r'^\d+,\d+: .*\$[^ ]+\.agc',line):break
  m=re.match(r'^\d+,(\d+):\s+([0-7]{2}),([0-7]{4})\s+([0-7]{5})(?:\s+([0-7]{5}))?',line)
  if not m or not lo<=int(m[1])<=hi:continue
  for index,raw in enumerate([m[4],m[5]]):
   if raw is None:continue
   address=int(m[3],8)+index;physical=int(m[2],8)*1024+address-1024
   words.append({'line':int(m[1]),'bank':m[2],'address':f'{address:04o}','physical_fixed_word':physical,'raw':raw})
   if address in range(0o3656 if mission==0 else 0o3655,0o3677 if mission==0 else 0o3676)or address==(0o3733 if mission==0 else 0o3730):rows.append([mission,physical,int(raw,8)])
 assert len([v for v in rows if v[0]==mission])==18
 result[name]={'original_source_sha256':record['original_sha256'],'assembled_source_sha256':assembled_sha,'support_rope_sha256':provenance['rope_sha256'],'source_audit_dependency':'evidence/'+audit,'source_edits':record.get('source_edits',0),'indentation_repairs':record.get('indentation_repair_lines',[]),'inverse_source_lines':[lo,hi],'inverse_emitted_words':len(words),'words':words}
(r/'build/inverse-trig-source-audit.json').write_text(json.dumps(result,indent=2)+'\n')
header='/* SPDX-License-Identifier: MIT\n * Byte-original INTERPRETER coefficient/degree/NEG12 values. */\n#ifndef INVERSE_TRIG_CONSTANTS_H\n#define INVERSE_TRIG_CONSTANTS_H\nstatic const uint16_t inverse_trig_constants[36][3]={\n'+''.join('{'+','.join(str(x)for x in row)+'},\n'for row in rows)+'};\n#endif\n';(r/'tests/inverse_trig_constants.h').write_text(header)
print('Original inverse source words',[v['inverse_emitted_words']for v in result.values()]);print('36 immutable constants mapped.')
