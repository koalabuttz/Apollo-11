#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,json,struct
r=pathlib.Path(__file__).resolve().parents[1];x=json.loads((r/'evidence/rtb-angles-source-map.json').read_text())['cm'];words=[]
for w in x['words']:
 if 325<=w['original_line']<=355:words.append(dict(w,native_group='QUALITY1_QUALITY2',status='qualified_native_source_phase'))
rope=struct.unpack('>36864H',(r/'deps/cm-rtb-fixture/MAIN.agc.bin').read_bytes());assert len(words)==25
for w in words:assert rope[w['physical_fixed_word']]>>1==int(w['raw_octal'],8)
# Original bank13 3754..3755 is the external immutable5/8 pair.
offset=0o13*1024+0o3754-1024;assert [rope[offset]>>1,rope[offset+1]>>1]==[0o24000,0]
result=dict(source_file=x['source_file'],original_sha256=x['original_sha256'],rope_sha256=x['rope_sha256'],source_audit_dependency='evidence/rtb-angles-source-audit.json',source_group_words=25,words=words,external_immutable_operand={'5/8':{'bank':'13','address':'3754','words':['24000','00000']}},scope='All local QUALITY1/QUALITY2 source words and literal inline operands; true saved ORBITAL QUALITY3 return023146. No external orbital algorithm or interpreter scheduling completion claim.')
(r/'build/cm-quality-source-map.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS25 original CM QUALITY words and external5/8 pair')
