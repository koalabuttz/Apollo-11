#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Original containing-word inventory with explicitly qualified angle groups."""
import pathlib,re,json,struct
r=pathlib.Path(__file__).resolve().parents[1];audit=json.loads((r/'build/rtb-angles-source-audit.json').read_text());result={}
qualified=['LOADTIME','CDULOGIC','1STO2S','V1STO2S','2V1STO2S','1TO2SUB','CDUINC','INCRCDUS','READPIPS']
for mission,name in [('lm','RTB_OP_CODES.agc'),('cm','RT8_OP_CODES.agc')]:
 source=(r/f'deps/{mission}-rtb-fixture'/audit[mission]['assembly_include']).read_text().splitlines();listing=(r/f'deps/{mission}-rtb-fixture/assembly.lst').read_text().split('$'+audit[mission]['assembly_include'],1)[1].split('\n',1)[1];words=[];group='outside_group';rope=struct.unpack('>36864H',(r/f'deps/{mission}-rtb-fixture/MAIN.agc.bin').read_bytes())
 for line in listing.splitlines():
  if re.match(r'^\d+,\d+: .*\$[^ ]+\.agc',line):break
  m=re.match(r'^\d+,(\d+):\s+([0-7]{2}),([0-7]{4})\s+([0-7]{5})(?:\s+([0-7]{5}))?',line)
  if not m:continue
  lineno=int(m[1]);text=source[lineno-1].split('#')[0]
  if text and text[0]not in' \t':
   label=text.split()[0]
   if label in qualified:group=label
   elif label not in ['TPMODE','VECMODE','LOCTHETA']:group='outside_group'
  for offset,value in enumerate([m[4]]+([m[5]]if m[5]else[])):
   address=int(m[3],8)+offset;bank=int(m[2],8);physical=bank*1024+address-1024
   assert rope[physical]>>1==int(value,8),(mission,lineno)
   words.append(dict(original_line=lineno,bank=m[2],address=f'{address:04o}',physical_fixed_word=physical,raw_octal=value,native_group=group,status='qualified_angles'if group!='outside_group'else'outside_this_packet'))
 result[mission]=dict(audit[mission],containing_emitted_words=len(words),qualified_emitted_words=sum(w['status']=='qualified_angles'for w in words),words=words,qualified_public_entries=[x for x in qualified if any(w['native_group']==x for w in words)],pending_containing_groups=['SGNAGREE','SIGNMPAC','NORMUNIT/NORMUNX1','VECSGNAG','PULSEIMU']+(['TRANSP1/TRANSP2','QUALITY1/QUALITY2']if mission=='cm'else[]),helper_boundary='SLOAD2/NEWMODE/VMODE actual source stores through entry DANZIG; direct TCQ completes without executing caller instruction')
(r/'build/rtb-angles-source-map.json').write_text(json.dumps(result,indent=2)+'\n');print({m:(x['qualified_emitted_words'],x['containing_emitted_words'])for m,x in result.items()})
