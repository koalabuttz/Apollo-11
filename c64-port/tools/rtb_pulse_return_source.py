#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,json,struct
r=pathlib.Path(__file__).resolve().parents[1];prior=json.loads((r/'evidence/rtb-angles-source-map.json').read_text());result={}
for mission in ['lm','cm']:
 x=prior[mission];address='3711'if mission=='lm'else'3657';selected=[w for w in x['words']if w['address']==address];assert len(selected)==1
 rope=struct.unpack('>36864H',(r/f'deps/{mission}-rtb-fixture/MAIN.agc.bin').read_bytes());w=selected[0];assert rope[w['physical_fixed_word']]>>1==int(w['raw_octal'],8);assert w['raw_octal']==('16061'if mission=='lm'else'16030')
 result[mission]=dict(source_file=x['source_file'],original_sha256=x['original_sha256'],rope_sha256=x['rope_sha256'],source_audit_dependency='evidence/rtb-angles-source-audit.json',source_group_words=1,words=selected,frontier='Original trailing PULSEIMU TCF DANZIG after external bank return. Preserved pending interpreter state; no DANZIG instruction fetched.')
(r/'build/rtb-pulse-return-source-map.json').write_text(json.dumps(result,indent=2)+'\n');print('Two original trailing PULSEIMU return words verified')
