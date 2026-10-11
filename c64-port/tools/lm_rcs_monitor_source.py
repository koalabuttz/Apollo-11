#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Map every byte-original RCS failure monitor emitted word."""
import pathlib,re,json,hashlib,struct
r=pathlib.Path(__file__).resolve().parents[1];source=r.parent/'Luminary099/RCS_FAILURE_MONITOR.agc'
audit=json.loads((r/'build/lm-rcs-monitor-source-audit.json').read_text());assert hashlib.sha256(source.read_bytes()).hexdigest()==audit['original_sha256']
listing=(r/'deps/lm-rcs-monitor-fixture/assembly.lst').read_text().split('$RCS_FAILURE_MONITOR.agc',1)[1].split('\n',1)[1]
words=[];constants={6816:0o2454,6817:0o40106}
m5=[0o40,0o20,0o100,0o200,0o10,1,4,2];m6=[0o10,0o20,4,0o200,1,2,0o40,0o100]
constants.update({6819+i:v for i,v in enumerate(m5)});constants.update({6827+i:v for i,v in enumerate(m6)})
for line in listing.splitlines():
 if re.match(r'^\d+,\d+: .*\$[^ ]+\.agc',line):break
 m=re.match(r'^\d+,(\d+):\s+([0-7]{2}),([0-7]{4})\s+([0-7]{5})(?:\s+([0-7]{5}))?',line)
 if not m:continue
 for offset,value in enumerate([m[4]]+([m[5]]if m[5]else[])):
  address=int(m[3],8)+offset;physical=int(m[2],8)*1024+address-1024
  group='containing_fixed_data'if physical in constants else('channel32_highest_change'if address<0o3203 else('valve_mask_update'if address<0o3236 else'NOVAC_admission_and_RESUME_handoff'))
  words.append(dict(original_line=int(m[1]),bank=m[2],address=f'{address:04o}',physical_fixed_word=physical,raw_octal=value,native_group=group))
assert len(words)==69,len(words)
rope=struct.unpack('>36864H',(r/'deps/lm-rcs-monitor-fixture/MAIN.agc.bin').read_bytes())
for w in words:assert rope[w['physical_fixed_word']]>>1==int(w['raw_octal'],8)
for a,v in constants.items():assert next(int(w['raw_octal'],8)for w in words if w['physical_fixed_word']==a)==v
x=dict(source_file='Luminary099/'+source.name,original_sha256=audit['original_sha256'],emitted_words=len(words),words=words,containing_constants=constants,source_entry='RCSMON06,3156 (RCSMONIT alias)',source_handoffs={'normal':'RESUME05270 entry, IRQ0 retained','resource_exhausted':'BAILOUT105716 entry before alarm1202 processing'},native_external_dependency='Source-qualified exec_admit NOVAC and actual fixed Q+2 helper return; no external RESUME/BAILOUT execution')
(r/'build/lm-rcs-monitor-source-map.json').write_text(json.dumps(x,indent=2)+'\n');print(len(words),'original source words mapped')
