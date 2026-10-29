#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Map every emitted original backup controller word and external constant aliases."""
import pathlib,re,json,hashlib,struct
r=pathlib.Path(__file__).resolve().parents[1];source=r.parent/'Luminary099/SPS_BACK-UP_RCS_CONTROL.agc'
audit=json.loads((r/'build/lm-sps-rcs-source-audit.json').read_text());assert hashlib.sha256(source.read_bytes()).hexdigest()==audit['original_sha256']
listing=(r/'deps/lm-sps-rcs-fixture/assembly.lst').read_text().split('$SPS_BACK-UP_RCS_CONTROL.agc',1)[1].split('\n',1)[1]
words=[];constants={18342:0o632,18372:4,18373:0o12,18374:0o12}
for line in listing.splitlines():
 if re.match(r'^\d+,\d+: .*\$[^ ]+\.agc',line):break
 m=re.match(r'^\d+,(\d+):\s+([0-7]{2}),([0-7]{4})\s+([0-7]{5})',line)
 if not m:continue
 lineno=int(m[1]);address=int(m[3],8);physical=int(m[2],8)*1024+address-1024
 group='source_constant'if physical in constants else('jet_sense_and_deadband'if 0o3707<=address<=0o3734 else('outer_rate_and_coast'if address>=0o3735 else'inhibition_and_store'))
 words.append(dict(original_line=lineno,bank=m[2],address=m[3],physical_fixed_word=physical,raw_octal=m[4],native_group=group))
raw=struct.unpack('>36864H',(r/'deps/lm-sps-rcs-fixture/MAIN.agc.bin').read_bytes())
for a,v in constants.items():assert next(int(w['raw_octal'],8)for w in words if w['physical_fixed_word']==a)==v
aliases={}
for name,address,value in [('RATELIM1',0o4766,0o32),('RATEDB1',0o4767,0o45)]:
 # yaYUL physical bank ordering: fixed-fixed banks2/3 precede bank0/1.
 offset=address-0o4000
 assert raw[offset]>>1==value,(name,raw[offset]>>1)
 aliases[name]=dict(source_alias='CALLCODE'if name=='RATELIM1'else'TBUILDFX',source_address=f'{address:05o}',raw_octal=f'{value:05o}')
x=dict(source_file='Luminary099/'+source.name,original_sha256=audit['original_sha256'],emitted_words=len(words),words=words,containing_constants=constants,external_fixed_aliases=aliases,source_entry='SPSRCS21,3727/E6',source_return='TCQ completes at saved fixed Q, Q becomes3. Internal labels have no external callers in original mission source.')
(r/'build/lm-sps-rcs-source-map.json').write_text(json.dumps(x,indent=2)+'\n');print(len(words),'original source words mapped')
