#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Original selected IMU stall/activation words, not whole-file credit."""
import pathlib,json,struct,re
r=pathlib.Path(__file__).resolve().parents[1];audit=json.loads((r/'evidence/imu-pulse-source-audit.json').read_text());out={}
for mission,a in audit.items():
 fixture=r/f'deps/{mission}-imu-pulse-fixture';rope=struct.unpack('>36864H',(fixture/'MAIN.agc.bin').read_bytes());listing=(fixture/'assembly.lst').read_text().split('$IMU_MODE_SWITCHING_ROUTINES.agc',1)[1].split('\n',1)[1];words=[]
 ranges=[(954,1066,'Standby local stages and data')]
 for line in listing.splitlines():
  if re.match(r'^\d+,\d+: .*\$[^ ]+\.agc',line):break
  m=re.match(r'^\d+,(\d+):\s+(?:([0-7]{2}),)?([0-7]{4})\s+([0-7]{5})(?:\s+([0-7]{5}))?',line)
  if not m:continue
  selected=[label for lo,hi,label in ranges if lo<=int(m[1])<=hi]
  if not selected:continue
  bank=int(m[2],8)if m[2]else int(m[3],8)//1024;physical=bank*1024+int(m[3],8)-1024 if m[2]else int(m[3],8)
  for extra,raw in enumerate([x for x in [m[4],m[5]]if x]):
   p=physical+extra;index=p^2048 if bank<4 else p;assert rope[index]>>1==int(raw,8)
   words.append(dict(original_line=int(m[1]),bank=f'{bank:02o}',address=f'{int(m[3],8)+extra:04o}',physical_fixed_word=p,raw_octal=raw,native_group=selected[0]))
 out[mission]=dict(source_file=a['source_file'],original_sha256=a['original_sha256'],assembled_sha256=a['assembled_sha256'],rope_sha256=a['rope_sha256'],source_audit_dependency='evidence/imu-pulse-source-audit.json',source_repairs=a['indentation_repairs'],source_group_words=len(words),words=words,containing_file_status='partial',compiled_boundaries=dict(FINETIME_plus1='04103'if mission=='lm'else'04554',UPFLAG='05504'if mission=='lm'else'05546',DOWNFLAG='05516'if mission=='lm'else'05560',BANKCALL='04616'if mission=='lm'else'04662',PHASCHNG='05353'if mission=='lm'else'05412',SHORTMP='07307'if mission=='lm'else'07256',TPAGREE='07257'if mission=='lm'else'07226',GOTOPOOH='06001'if mission=='lm'else'04106'),remaining=['External FINETIME/SHORTMP/TPAGREE/flag/phase/bank/display bodies','Physical standby transition and live scaler producers'])
(r/'build/imu-standby-source-map.json').write_text(json.dumps(out,indent=2)+'\n');print({m:a['source_group_words']for m,a in out.items()})


values=[]
for mission in ['lm','cm']:
 selected=[w for w in out[mission]['words'] if w['original_line'] in [955,967,970,972,979,980,982,986,1064]]
 assert len(selected)==10
 values.append(' {'+','.join('{0%o,0%s}'%(w['physical_fixed_word'],w['raw_octal']) for w in selected)+'},')
(r/'include/imu_standby_literals.h').write_text('/* SPDX-License-Identifier: MIT\n * Original local passive fixed words, generated from the listing. */\nstatic const struct {uint16_t address;agc_word value;} standby_literals[2][10]={\n'+'\n'.join(values)+'\n};\n')
