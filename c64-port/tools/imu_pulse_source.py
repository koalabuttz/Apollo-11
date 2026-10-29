#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Retain selected original IMUPULSE emitted words, with actual bank07."""
import pathlib,json,struct,re
r=pathlib.Path(__file__).resolve().parents[1];audit=json.loads((r/'build/imu-pulse-source-audit.json').read_text());out={}
for mission,a in audit.items():
 fixture=r/f'deps/{mission}-imu-pulse-fixture';rope=struct.unpack('>36864H',(fixture/'MAIN.agc.bin').read_bytes());listing=(fixture/'assembly.lst').read_text().split('$IMU_MODE_SWITCHING_ROUTINES.agc',1)[1].split('\n',1)[1];words=[]
 ranges=[(424,475,'IMUPULSE/GWAKE2/GYROAGRE/GYROBUSY/REGSLEEP'),(487,487,'LGWAKE'),(704,711,'CAGETSTJ'),(103,104,'MODEEXIT')] if mission=='lm' else [(428,479,'IMUPULSE/GWAKE2/GYROAGRE/GYROBUSY/REGSLEEP'),(491,491,'LGWAKE'),(708,715,'CAGETSTJ'),(104,105,'MODEEXIT')]
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
 out[mission]=dict(**a,source_group_words=len(words),words=words,containing_file_status='partial',compiled_boundaries=dict(entry='03323'if mission=='lm'else'03206',WAITLIST='05203'if mission=='lm'else'05245',WAITLIST_return='03337'if mission=='lm'else'03222',JOBSLEEP='05133'if mission=='lm'else'05175',successful_return='BUF2 caller, Q0134, savedFB/oldEB, IRQ1'),helpers=['immutable source-qualified WAITLIST canonical adapter','TPAGREE arithmetic limit/carry chain','SWRETURN body exchange'],remaining=['GWAKE resumed sleeper','JOBSLEEP scheduling and waking lifecycle','STRTGYRO/GSELECT/GYROCMD deferred hardware tasks','other containing-file entries'])
(r/'build/imu-pulse-source-map.json').write_text(json.dumps(out,indent=2)+'\n');print({m:a['source_group_words']for m,a in out.items()})
