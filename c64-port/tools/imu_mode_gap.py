#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Original whole IMU source footprint and existing native group gap inventory."""
import pathlib,json,hashlib
from fixed_data_generate import listing_records,source_line,rope_word
r=pathlib.Path(__file__).resolve().parents[1];audit=json.loads((r/'evidence/imu-pulse-source-audit.json').read_text());maps={name:json.loads((r/f'evidence/{name}-source-map.json').read_text())for name in ['imu-pulse','imu-pulse-task','imu-pulse-end','imu-pulse-wake','imu-coarse']};result={}
for mission,a in audit.items():
 path=a['source_file'];original=(r.parent/path).read_text();fixture=r/f'deps/{mission}-imu-pulse-fixture';rope=(fixture/'MAIN.agc.bin').read_bytes();assert hashlib.sha256(rope).hexdigest()==a['rope_sha256'];assert hashlib.sha256(original.encode()).hexdigest()==a['original_sha256']
 records=listing_records(fixture/'assembly.lst',{'IMU_MODE_SWITCHING_ROUTINES.agc':path},{path:original},rope);owners={}
 for packet,x in maps.items():
  for w in x[mission]['words']:owners.setdefault(w['physical_fixed_word'],[]).append({'packet':f'evidence/{packet}-proof.json','group':w['native_group']})
 label='';labels={}
 for n,line in enumerate(original.splitlines(),1):
  parsed=source_line(line)
  if parsed and parsed.get('label')and parsed.get('op')not in ['=','EQUALS']and parsed['label'][0]not in '+-':label=parsed['label']
  labels[n]=label
 words=[];groups={}
 for row in records:
  for i,word in enumerate(row['words']):
   at=row['physical_fixed_word']+i;assert rope_word(rope,at)==word
   x={'line':row['line'],'label':row['label'],'op':row['op'],'arguments':row['arguments'],'address':row['assembled_address'],'physical_fixed_word':at,'raw_octal':f'{word:05o}','current_local_group':labels[row['line']],'native_owners':owners.get(at,[])};words.append(x)
   if not x['native_owners']:groups.setdefault(x['current_local_group'],[]).append(at)
 assert len({w['physical_fixed_word']for w in words})==len(words)
 result[mission]={'source_file':path,'source_audit':'evidence/imu-pulse-source-audit.json','original_sha256':a['original_sha256'],'assembled_sha256':a['assembled_sha256'],'semantic_edits':0,'indentation_repairs':a['indentation_repairs'],'rope_sha256':a['rope_sha256'],'original_emitted_words':len(words),'native_covered_words':sum(bool(w['native_owners'])for w in words),'remaining_words':sum(not w['native_owners']for w in words),'words':words,'remaining_local_groups':{g:{'words':len(v),'physical_words':v}for g,v in groups.items()},'status':'Partial containing source. Existing complete numeric/service proofs do not imply whole-file or application coverage.'}
 print(mission,len(words),'original words;',result[mission]['native_covered_words'],'native;',result[mission]['remaining_words'],'remaining;',len(groups),'local groups')
(r/'build/imu-mode-gap-map.json').write_text(json.dumps(result,indent=2)+'\n')
