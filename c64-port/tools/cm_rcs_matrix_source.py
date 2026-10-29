#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Review each emitted word of the byte-original RCS update source."""
import pathlib,re,json,hashlib
r=pathlib.Path(__file__).resolve().parents[1];source=r.parent/'Comanche055/RCS-CSM_DAP_EXECUTIVE_PROGRAMS.agc';audit=json.loads((r/'build/cm-rcs-matrix-source-audit.json').read_text());assert hashlib.sha256(source.read_bytes()).hexdigest()==audit['original_sources'][source.name]['original_sha256'];listing=(r/'deps/cm-rcs-matrix-fixture/assembly.lst').read_text().split('$RCS-CSM_DAP_EXECUTIVE_PROGRAMS.agc',1)[1].split('\n',1)[1];words=[]
for line in listing.splitlines():
 if re.match(r'^\d+,\d+: .*\$[^ ]+\.agc',line):break
 m=re.match(r'^\d+,(\d+):\s+([0-7]{2}),([0-7]{4})\s+([0-7]{5})',line)
 if not m:continue
 lineno=int(m[1]);group='RCS_gates'if int(m[3],8)<=0o3454 else('source_quad_constant'if 'QUADANGL' in line and 'DEC' in line else'source_matrix_update')
 words.append({'original_line':lineno,'bank':m[2],'address':m[3],'physical_fixed_word':int(m[2],8)*1024+int(m[3],8)-1024,'raw_octal':m[4],'native_group':group})
assert len(words)==33,len(words)
assert words[-1]['raw_octal']=='01224'and words[-1]['physical_fixed_word']==19271
x={'source_file':'Comanche055/'+source.name,'original_sha256':audit['original_sources'][source.name]['original_sha256'],'emitted_words':len(words),'words':words,'external_trig_aliases':audit['native_aliases'],'source_return':'All three source exits reach actual ENDOFJOB05217 with source A/L/Q/banks retained; this module stops before external Executive job completion.'};(r/'build/cm-rcs-matrix-source-map.json').write_text(json.dumps(x,indent=2)+'\n');print(len(words),'original source words mapped')
