#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Exact original antenna plane/projection/UNIT/BOV source and passive literal map."""
import hashlib,json,pathlib
from fixed_data_generate import listing_records,symbols,fixed_address,rope_word
R=pathlib.Path(__file__).resolve().parents[1]
def main():
 result={};audits=json.loads((R/'build/sband-source/audit.json').read_text())
 for mission,name in [('LM','Luminary099'),('CM','Comanche055')]:
  work=R/'build/sband-source'/name;sy=symbols((work/'assembly.lst').read_text());rope=(work/'MAIN.agc.bin').read_bytes();audit=next(x for x in audits if x['source_path'].startswith(name+'/'))
  lo,hi=(0o3671,0o3701)if mission=='LM'else(0o3714,0o3724)
  selected=[x for x in audit['source_line_word_map']if x['words']and lo<=int(x['assembled_address'].split(',')[-1],8)<hi]
  assert sum(len(x['words'])for x in selected)==8
  label='HIUNITY'if mission=='LM'else'HIUNITZ';at=fixed_address(sy[label]);words=[rope_word(rope,at+i)for i in range(6)];assert words==([0,0,0o20000,0,0,0]if mission=='LM'else[0,0,0,0,0o20000,0])
  original='INTERPRETIVE_CONSTANT.agc'if mission=='LM'else'INTERPRETIVE_CONSTANTS.agc';source=R.parent/name/original;fixture=work/'INTERPRETIVE_CONSTANTS.agc';assert fixture.read_bytes()==source.read_bytes()
  source_path=f'{name}/{original}';records=listing_records(work/'assembly.lst',{'INTERPRETIVE_CONSTANTS.agc':source_path},{source_path:source.read_text()},rope);data=[x for x in records if x['words']and x['physical_fixed_word']is not None and at<=x['physical_fixed_word']<at+6];assert sum((x['words']for x in data),[])==words
  result[mission]=dict(source_path=audit['source_path'],source_sha256=audit['source_sha256'],rope_sha256=audit['rope_sha256'],source_edits=0,containing_file_status='partial',native_source='src/sband_plane.c',source_words=8,source_line_word_map=selected,literal=dict(label=label,address=sy[label],physical_fixed_word=at,words=words,source_path=source_path,source_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),source_line_word_map=data,usage='All six original immutable vector words consumed by actual VPROJ; no padded instruction data.'),bindings={k:sy[k]for k in ['NEWOPS','DANZIG','UNIT','SQRTABRT','PDVL','VPROJ','BVSU','BOV(B)']},scope='Actual pending PDVL followed by VPROJ/VSL2/BVSU/BOV and full UNIT/BOV; stops before next actual NEWOPS operator or before SQRTABRT. Null LM EXIT and CM pitch DOT remain pending.',alias_contract='Actual PDVL stack exchange preserves old destination Z minor in L; R and stack are distinct in these exact caller layouts. Private compiled BVSU reads stable R in Y/Z/X source order and preserves DAS/OVERFLOW scratch. All arithmetic uses one canonical2048-word image.',caller_controls='Original pendingDANZIG LOC3670LM/3713CM EDOP062; sourceBANKSET064000+EB; FB0 and coherentEBBB, MODE-1, fiveVAC/PUSHbaseLM/base+010CM, NEWJOB+0, SUPER100/120/140/160 and producer exclusion.',coverage='65536inputs per mission: raw16A/L, ordinaryQ and ordinary initialOVFIND marginals; fiveVAC/eightEB/fourSUPERforms/bothIRQ; 4080 additional modest coherent R vectors plus axis/signedzero cases. 4090LM/4089CM normal,61446LM/61447CM null-exit,zero1302 samples. Not exhaustive Cartesian or all alarm edges.')
 (R/'build/sband-plane-source-map.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS both eight-word antenna plane maps and six-word immutable source literals')
if __name__=='__main__':main()
