#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Original selected IMU initialization/PIPA words, not whole-file credit."""
import pathlib,json,struct,re
r=pathlib.Path(__file__).resolve().parents[1];audit=json.loads((r/'evidence/imu-pulse-source-audit.json').read_text());out={}
for mission,a in audit.items():
 fixture=r/f'deps/{mission}-imu-pulse-fixture';rope=struct.unpack('>36864H',(fixture/'MAIN.agc.bin').read_bytes());listing=(fixture/'assembly.lst').read_text().split('$IMU_MODE_SWITCHING_ROUTINES.agc',1)[1].split('\n',1)[1];words=[]
 ranges=[(42,46,'ZEROICDU'),(58,101,'IMUZERO'),(106,128,'zero tasks'),(133,142,'IMUCOARS'),(312,335,'IMUFINE'),(341,415,'fail/PIPA/NOATTOFF'),(698,702,'called CAGETSTQ'),(838,838,'BITS4-5')]if mission=='lm'else[(42,46,'ZEROICDU'),(59,102,'IMUZERO'),(107,129,'zero tasks'),(134,143,'IMUCOARS'),(316,339,'IMUFINE'),(345,419,'fail/PIPA/NOATTOFF'),(702,706,'called CAGETSTQ'),(838,838,'BITS4-5')]
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
 out[mission]=dict(source_file=a['source_file'],original_sha256=a['original_sha256'],assembled_sha256=a['assembled_sha256'],rope_sha256=a['rope_sha256'],source_audit_dependency='evidence/imu-pulse-source-audit.json',source_repairs=a['indentation_repairs'],source_group_words=len(words),words=words,containing_file_status='partial',compiled_boundaries=dict(VARDELAY='05224'if mission=='lm'else'05272',FIXDELAY='05221'if mission=='lm'else'05267',SENDPULS='03133'if mission=='lm'else'03016',ENDIMU='03631'if mission=='lm'else'03514',ALARM='05567'if mission=='lm'else'05650',IMUBAD='03637'if mission=='lm'else'03522'),remaining=['External WAITLIST/VARDELAY/ALARM/SETISSW callee bodies','SENDPULS channel14 activation/asynchronous CDU transport','STALL/attitude/R02/standby remaining local groups'])
(r/'build/imu-mode-control-source-map.json').write_text(json.dumps(out,indent=2)+'\n');print({m:a['source_group_words']for m,a in out.items()})

# Reproduce the private passive-literal table from the original listing.
literal_lines={'lm':[65,93,101,126,140,328,333,357,397,411,838],'cm':[66,94,102,127,141,332,337,361,401,415]}
rows=[]
for mission in ['lm','cm']:
 values=[w for w in out[mission]['words'] if w['original_line'] in literal_lines[mission]]
 assert len(values)==(15 if mission=='lm' else 14)
 row=','.join('{0%o,0%s}'%(w['physical_fixed_word'],w['raw_octal']) for w in values)
 if len(values)==14:row+=',{0,0}'
 rows.append(' {'+row+'},')
header='/* SPDX-License-Identifier: MIT\n * Original local inline data, source-map verified. */\nstatic const struct {uint16_t address;agc_word value;} mode_literals[2][15]={\n'+'\n'.join(rows)+'\n};\n'
(r/'include/imu_mode_control_literals.h').write_text(header)
