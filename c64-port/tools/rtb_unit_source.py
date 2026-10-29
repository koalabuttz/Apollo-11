#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Original UNIT/UNITDV source word qualification for both mission callers."""
import pathlib,json,hashlib,re,struct
r=pathlib.Path(__file__).resolve().parents[1];front=json.loads((r/'evidence/rtb-vector-source-map.json').read_text());out={}
for mission,x in front.items():
 tree='Luminary099'if mission=='lm'else'Comanche055';source=r.parent/tree/'INTERPRETER.agc';fixture=r/f'deps/{mission}-rtb-fixture';rope=struct.unpack('>36864H',(fixture/'MAIN.agc.bin').read_bytes());record=x['interpreter_helpers'];assert hashlib.sha256(source.read_bytes()).hexdigest()==record['original_sha256'];assert hashlib.sha256((fixture/'INTERPRETER.agc').read_bytes()).hexdigest()==record['assembled_sha256'];words=[]
 listing=(fixture/'assembly.lst').read_text().split('$INTERPRETER.agc',1)[1].split('\n',1)[1]
 ranges=[(2194,2333,'UNIT/UNITDV'),(2465,2490,'VSQSUB')]if mission=='lm'else[(2203,2341,'UNIT/UNITDV'),(2473,2498,'VSQSUB')]
 for line in listing.splitlines():
  if re.match(r'^\d+,\d+: .*\$[^ ]+\.agc',line):break
  m=re.match(r'^\d+,(\d+):\s+(?:([0-7]{2}),)?([0-7]{4})\s+([0-7]{5})',line)
  if not m:continue
  selected=[name for lo,hi,name in ranges if lo<=int(m[1])<=hi]
  if not selected:continue
  bank=int(m[2],8)if m[2]else int(m[3],8)//1024;p=bank*1024+int(m[3],8)-1024 if m[2]else int(m[3],8);index=p^2048 if bank<4 else p;assert rope[index]>>1==int(m[4],8)
  words.append(dict(original_line=int(m[1]),bank=f'{bank:02o}',address=m[3],physical_fixed_word=p,raw_octal=m[4],native_group=selected[0]))
 out[mission]=dict(source_file=tree+'/INTERPRETER.agc',original_sha256=record['original_sha256'],assembled_sha256=record['assembled_sha256'],rope_sha256=x['rope_sha256'],audit_dependency=record['audit_dependency'],source_repairs=record['indentation_repairs'],source_group_words=len(words),words=words,containing_file_status='partial',RTB_caller_dependency='evidence/rtb-vector-proof.json',immutable_numeric_dependencies=['src/guidance_length_phased.c','src/guidance_unit_phased.c','src/guidance_norm.c'],source_helpers=['VECAGREE','MPACVBUF','DSQSUB','VSQSUB','SQRTSUB','UNITDV','GENDDV','DVOVF','VROTATEX/VMODE'],scope='Full original UNIT and NORM caller numerical return, initialized valid VAC/vector mode. No complete INTERPRETER source-file claim.')
(r/'build/rtb-unit-source-map.json').write_text(json.dumps(out,indent=2)+'\n');print({m:x['source_group_words']for m,x in out.items()})
