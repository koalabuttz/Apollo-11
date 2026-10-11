#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Exact RTB vector/frontier footprint and original VECAGREE helper words."""
import pathlib,json,hashlib,re,struct
r=pathlib.Path(__file__).resolve().parents[1];first=json.loads((r/'evidence/rtb-angles-source-map.json').read_text());out={}
for mission,x in first.items():
 selected=[]
 for old in x['words']:
  w=dict(old);a=int(w['address'],8)
  if w['bank'] != ('10' if mission=='lm' else '22'):continue
  if 0o3724<=a<=0o3771:group='NORMUNIT/NORMUNX1_to_UNIT+1'
  elif 0o3772<=a<=0o3774:group='VECSGNAG'
  elif (mission=='lm'and 0o3705<=a<=0o3710)or(mission=='cm'and 0o3653<=a<=0o3656):group='PULSEIMU_to_IMUPULSE'
  else:continue
  w['native_group']=group;w['status']='qualified_vector_frontier';selected.append(w)
 tree='Luminary099'if mission=='lm'else'Comanche055';auditpath='evidence/'+('lm-powered-source-audit.json'if mission=='lm'else'cm-powered-trig-source-audit.json');audit=json.loads((r/auditpath).read_text());record=audit['original_sources']['INTERPRETER.agc'];source=r.parent/tree/'INTERPRETER.agc';fixture=r/f'deps/{mission}-rtb-fixture'
 assert hashlib.sha256(source.read_bytes()).hexdigest()==record['original_sha256']
 assert hashlib.sha256((fixture/'INTERPRETER.agc').read_bytes()).hexdigest()==record.get('assembled_source_sha256',record.get('normalized_sha256'))
 rope=struct.unpack('>36864H',(fixture/'MAIN.agc.bin').read_bytes());listing=(fixture/'assembly.lst').read_text().split('$INTERPRETER.agc',1)[1].split('\n',1)[1];helpers=[]
 ranges=[(1324,1347),(2179,2189)]if mission=='lm'else[(1332,1355),(2188,2198)]
 for line in listing.splitlines():
  if re.match(r'^\d+,\d+: .*\$[^ ]+\.agc',line):break
  m=re.match(r'^\d+,(\d+):\s+(?:([0-7]{2}),)?([0-7]{4})\s+([0-7]{5})',line)
  if not m or not any(lo<=int(m[1])<=hi for lo,hi in ranges):continue
  bank=int(m[2],8)if m[2]else(int(m[3],8)//1024);p=bank*1024+int(m[3],8)-1024 if m[2]else int(m[3],8);binary_index=p^2048 if bank<4 else p;assert rope[binary_index]>>1==int(m[4],8),(mission,m.groups(),p,oct(rope[binary_index]>>1))
  helpers.append(dict(original_line=int(m[1]),bank=f'{bank:02o}',address=m[3],physical_fixed_word=p,raw_octal=m[4]))
 out[mission]=dict(source_file=x['source_file'],original_sha256=x['original_sha256'],rope_sha256=x['rope_sha256'],qualified_containing_words=len(selected),words=selected,containing_status='partial',interpreter_helpers=dict(source_file=tree+'/INTERPRETER.agc',original_sha256=record['original_sha256'],assembled_sha256=hashlib.sha256((fixture/'INTERPRETER.agc').read_bytes()).hexdigest(),audit_dependency=auditpath,indentation_repairs=record.get('indentation_repair_lines',[]),words=helpers),actual_boundaries=dict(VECAGREE='TC A before fixed caller',VECSGNAG='DANZIG entry; Q03775',NORMUNIT='UNIT+1 PC03024, FB0 Q01024',PULSEIMU='IMUPULSE PC03323 LM /03206 CM, FB014000; helper remains external'))
(r/'build/rtb-vector-source-map.json').write_text(json.dumps(out,indent=2)+'\n');print({m:(x['qualified_containing_words'],len(x['interpreter_helpers']['words']))for m,x in out.items()})
