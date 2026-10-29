#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source word map for deferred gyro segment body and actual output frontiers."""
import pathlib,json,struct,re
r=pathlib.Path(__file__).resolve().parents[1];audit=json.loads((r/'evidence/imu-pulse-source-audit.json').read_text());out={}
for mission,a in audit.items():
 fixture=r/f'deps/{mission}-imu-pulse-fixture';rope=struct.unpack('>36864H',(fixture/'MAIN.agc.bin').read_bytes());listing=(fixture/'assembly.lst').read_text().split('$IMU_MODE_SWITCHING_ROUTINES.agc',1)[1].split('\n',1)[1];words=[]
 ranges=[(477,485,'GWAKE restore/resleep'),(487,487,'LGWAKE')] if mission=='lm' else [(481,489,'GWAKE restore/resleep'),(491,491,'LGWAKE')]
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
 out[mission]=dict(source_file=a['source_file'],original_sha256=a['original_sha256'],assembled_sha256=a['assembled_sha256'],rope_sha256=a['rope_sha256'],source_audit_dependency='evidence/imu-pulse-source-audit.json',source_repairs=a['indentation_repairs'],source_group_words=len(words),words=words,containing_file_status='partial',compiled_boundaries=dict(GWAKE='03374'if mission=='lm'else'03257',JOBSLEEP='05133'if mission=='lm'else'05175',WAITLIST='05203'if mission=='lm'else'05245',WAITLIST_return='03337'if mission=='lm'else'03222',idle_ADVAN='03214'if mission=='lm'else'03231'),remaining=['selected basic or interpreted job body','TASKOVER scheduler/interrupt continuation','asynchronous gyro pulse delivery and active-channel augmentation backend','other containing-file entries'])
(r/'build/imu-pulse-wake-source-map.json').write_text(json.dumps(out,indent=2)+'\n');print({m:a['source_group_words']for m,a in out.items()})
