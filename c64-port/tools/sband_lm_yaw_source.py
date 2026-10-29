#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Exact LM antenna yaw statements and immutable source literal maps."""
import hashlib,json,pathlib
from fixed_data_generate import symbols,fixed_address,rope_word,listing_records
R=pathlib.Path(__file__).resolve().parents[1]
def main():
 work=R/'build/sband-source/Luminary099';sy=symbols((work/'assembly.lst').read_text());rope=(work/'MAIN.agc.bin').read_bytes();audit=next(x for x in json.loads((R/'build/sband-source/audit.json').read_text())if x['source_path'].startswith('Luminary099/'))
 selected=[x for x in audit['source_line_word_map']if x['words']and 0o3723<=int(x['assembled_address'].split(',')[-1],8)<0o3745];assert sum(len(x['words'])for x in selected)==18
 path='Luminary099/INTERPRETIVE_CONSTANT.agc';original=R.parent/path;assert original.read_bytes()==(work/'INTERPRETIVE_CONSTANTS.agc').read_bytes();records=listing_records(work/'assembly.lst',{'INTERPRETIVE_CONSTANTS.agc':path},{path:original.read_text()},rope)
 constants={}
 for label,count,expected in [('HIUNITZ',6,[0,0,0,0,0o20000,0]),('HIUNITX',6,[0o20000,0,0,0,0,0])]:
  at=fixed_address(sy[label]);words=[rope_word(rope,at+i)for i in range(count)];assert words==expected;data=[x for x in records if x['words']and x['physical_fixed_word']is not None and at<=x['physical_fixed_word']<at+count];assert sum((x['words']for x in data),[])==words;constants[label]=dict(address=sy[label],physical_fixed_word=at,words=words,source_line_word_map=data)
 result=dict(source_path=audit['source_path'],source_sha256=audit['source_sha256'],rope_sha256=audit['rope_sha256'],source_edits=0,containing_file_status='partial',native_source='src/sband_lm_yaw.c',source_words=18,source_line_word_map=selected,constants=constants,constant_source_path=path,constant_source_sha256=hashlib.sha256(original.read_bytes()).hexdigest(),scope='Complete original LM yaw calculation from returned-pitch NOADJUST NEWOPS3723; ends before original EXIT atNEWOPS3745 or source1301/1302 service. Remaining antenna basic control remains separate.',alias_contract='VLOAD UR andliveVXV URP preserve canonicalVAC vectors; STODL savesRLM andloadsyaw; vectorPDDL pushesUZsin whileloadingpitch andleaves oldstackXminor/Ypair inMPAC; implicitpop/VSU restoresstack; DOT/PDVL/ABVAL/SIGN implicitpop retain actualsignpair. FinalSTOREYAWANG leavesold destinationminorL.',caller_controls='ActualNEWOPS FB/BANKSET064000+EB, SUPER100/120/140/160, coherentEBBB, NEWJOB+0, ordinaryRAM/Q/rawA/L, MODE0 fiveVAC andPUSHbase+014.',coverage='65536inputs with complete rawA/L/ordinaryQ marginals, variedvectors/scalars/signedzeros, fiveVAC/eightEB/fourSUPERforms/bothIRQ. NotexhaustiveCartesian.')
 (R/'build/sband-lm-yaw-source-map.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS original LM18word yaw map plustwo exact literal objects')
if __name__=='__main__':main()
