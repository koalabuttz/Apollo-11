#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Original leading antenna pairs and the explicit compiler-bound controls."""
import hashlib,json,pathlib
from fixed_data_generate import symbols,fixed_address,rope_word
R=pathlib.Path(__file__).resolve().parents[1]
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 audits=json.loads((R/'build/sband-source/audit.json').read_text());result={}
 for mission,name in [('LM','Luminary099'),('CM','Comanche055')]:
  work=R/'build/sband-source'/name;sy=symbols((work/'assembly.lst').read_text());rope=(work/'MAIN.agc.bin').read_bytes();a=next(x for x in audits if x['source_path'].startswith(name+'/'))
  def logical(k):return int(sy[k].split(',')[-1],8)
  def cadr(k):return fixed_address(sy[k])
  conic='LEMCONIC' if mission=='LM'else 'CSMCONIC';first=logical('SBANDANT')+3
  sload=next(r for r in a['source_line_word_map']if r['op']=='SLOAD')
  selected=[r for r in a['source_line_word_map']if r['words'] and r['physical_fixed_word'] is not None and fixed_address(sy['SBANDANT'])+3<=r['physical_fixed_word']<sload['physical_fixed_word']]
  expected={'LM':(0o3611,0o2453,0o21573,0o27100,0o7613,0o6640),'CM':(0o3655,0o2500,0o45510,0o27114,0o7562,0o6607)}[mission]
  assert (first,logical('RTB')+3,cadr('LOADTIME'),cadr(conic),logical('SETPD'),logical('CALL'))==expected
  assert rope_word(rope,fixed_address(sy['BANKMASK']))==0o76000
  result[mission]=dict(source_path=a['source_path'],source_sha256=a['source_sha256'],rope_sha256=a['rope_sha256'],source_edits=0,native_source='src/sband_flow.c',scope='Explicit leading antenna pairs through true interpreted orbital handoff. CM joined caller executes frozen CDUTRIG and genuine RVQ before STCALL CSMCONIC. External conic bodies not executed.',containing_file_status='partial',source_line_word_map=selected,original_bindings={k:sy[k]for k in ['SBANDANT','LOADTIME',conic,'CDUTRIG','NEWOPS','DANZIG','RTB','SWCALL','SWRETURN','SETPD','CALL','BANKMASK','TDEC1','QPRET','PUSHLOC']},compiled_tuple=expected,dependencies=['include/rtb_angles.h','include/cm_powered_interface.h','include/cm_powered_trig.h'],caller_domain='Five valid VAC bases; NEWJOB+0; coherent original banks; ordinary image/raw16 A/L/Q; producer exclusion throughout all composed phases.')
 (R/'build/sband-flow-source-map.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS original antenna leading word maps',[(m,sum(len(x['words'])for x in v['source_line_word_map']))for m,v in result.items()])
if __name__=='__main__':main()
