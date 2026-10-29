#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Actual original antenna zero-angle setup/SMNB caller and source zero data."""
import hashlib,json,pathlib
from fixed_data_generate import listing_records,symbols,fixed_address,rope_word,physical_erasable
R=pathlib.Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 result={};audits=json.loads((R/'build/sband-source/audit.json').read_text())
 for mission,name in [('LM','Luminary099'),('CM','Comanche055')]:
  work=R/'build/sband-source'/name;sy=symbols((work/'assembly.lst').read_text());rope=(work/'MAIN.agc.bin').read_bytes();a=next(x for x in audits if x['source_path'].startswith(name+'/'))
  lo,hi=(0o3646,0o3654)if mission=='LM'else(0o3704,0o3712)
  selected=[x for x in a['source_line_word_map']if x['words']and lo<=int(x['assembled_address'].split(',')[-1],8)<hi]
  filename='INTERPRETIVE_CONSTANT.agc'if mission=='LM'else'INTERPRETIVE_CONSTANTS.agc';original=R.parent/name/filename;at=fixed_address(sy['HI6ZEROS']);words=[rope_word(rope,at+i)for i in range(6)];assert words==[0]*6 and original.read_bytes()==(work/'INTERPRETIVE_CONSTANTS.agc').read_bytes()
  records=listing_records(work/'assembly.lst',{'INTERPRETIVE_CONSTANTS.agc':name+'/'+filename},{name+'/'+filename:original.read_text()},rope);zero_rows=[x for x in records if x['words']and x['physical_fixed_word']is not None and at<=x['physical_fixed_word']<at+6];assert sum((x['words']for x in zero_rows),[])==words
  count_at=fixed_address(sy['NO.WDS'])-1; counts=[rope_word(rope,count_at+i)for i in range(3)];assert counts==[6,2,3]
  if mission=='LM':assert physical_erasable(sy['PITCHANG'])==0o2200 and physical_erasable(sy['YAWANG'])==0o2202
  result[mission]=dict(source_path=a['source_path'],source_sha256=a['source_sha256'],rope_sha256=a['rope_sha256'],source_edits=0,containing_file_status='partial',native_source='src/sband_bodyframe.c',scope='Actual post-frame-shift PUSH/PDDL, zero angle STORE/STOVL, CALL *SMNB*, complete unchanged AX/interface/VLOAD/RVQ helpers; stops at original caller NEWOPS before following STORE/STODL.',source_line_word_map=selected,source_words=sum(len(x['words'])for x in selected),constants=dict(source_path=name+'/'+filename,original_sha256=sha(original),assembled_sha256=sha(work/'INTERPRETIVE_CONSTANTS.agc'),physical_fixed_word=at,words=words,source_line_word_map=zero_rows,fixture_filename='INTERPRETIVE_CONSTANTS.agc; unchanged original content under fixture plural filename',usage='Only first DPzero pair fetched as compiled native zero values. Remaining4 words retained passive source table provenance, no algorithm credit.'),bindings={k:sy[k]for k in ['HI6ZEROS','*SMNB*','AX*SR*T','NEWOPS','DANZIG','SINCDU','COSCDU']},push_counts=dict(source_path=name+'/INTERPRETER.agc',original_sha256=sha(R.parent/name/'INTERPRETER.agc'),physical_fixed_word=count_at,words=counts,usage='Vector6 used by these compiled PUSH/PDDL/pop tuples; adjacentDP2/TP3 table words retained passive only.'),alias_contract='LM PUSH saves vector at VACbase then VLOAD pops it; final L is originalMPAC.Z-low. CM PDDL saves at VACbase+2 and loads zeroDP, moving old stackY intoMPAC.Z; subsequentRCM reload leaves oldstack+3 inL. Only canonical2048 image is authoritative.',caller_controls='Valid5VAC and exact sourcePUSHLOC baseLM/base+2CM; bank0 pendingDANZIG, LOC3645LM/3704CM, EDOP0LM/052CM, sourceBANKSET64bank withSUPERbit100, MODE-1, NEWJOB+0, producer exclusion through all callee returns.',coverage='65536cases/mission, everyraw16A/L and ordinaryQ marginal, all5VAC/all8EB/four bank42SUPERforms/bothIRQ, sampled vector/trig inputs and signed-zero corners. No Cartesian/allinstruction-edge/physicalprecision guarantee beyond original AXcontract.')
 (R/'build/sband-bodyframe-source-map.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS body-frame source maps',[(m,x['source_words'])for m,x in result.items()])
if __name__=='__main__':main()
