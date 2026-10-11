#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Audit exact original SHORTV/VSSL and overflow-helper instruction/data words."""
import hashlib,json,pathlib
from fixed_data_generate import listing_records,symbols,fixed_address,rope_word
R=pathlib.Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 result={}
 for mission,name in [('LM','Luminary099'),('CM','Comanche055')]:
  work=R/'build/sband-source'/name;original=R.parent/name/'INTERPRETER.agc';sy=symbols((work/'assembly.lst').read_text());rope=(work/'MAIN.agc.bin').read_bytes()
  def address(label):return fixed_address(sy[label])
  intervals=[(address('SHORTV'),address('SHORTV')+5),(address('VSSL')-1,address('TSLC2')),(address('OVERFLWZ'),address('BVSU')),(address('SETOVF2'),address('SETOVF2')+2)]
  records=listing_records(work/'assembly.lst',{'INTERPRETER.agc':name+'/INTERPRETER.agc'},{name+'/INTERPRETER.agc':original.read_text()},rope)
  selected=[x for x in records if x['words'] and x['physical_fixed_word'] is not None and any(a<=x['physical_fixed_word']<b for a,b in intervals)]
  assert sy['SHORTV']=='00,2121'and sy['VSSL']=='00,2145'
  constants={k:dict(address=sy[k],physical_fixed_word=address(k),word=rope_word(rope,address(k)))for k in ['LOW3','ZERO','THREE','FIVE','ONE','LIMITS','POSMAX','NEGMAX']}
  assert constants['LOW3']['word']==7 and constants['THREE']['word']==3 and constants['FIVE']['word']==5
  result[mission]=dict(source_path=name+'/INTERPRETER.agc',original_sha256=sha(original),assembled_sha256=sha(work/'INTERPRETER.agc'),rope_sha256=sha(work/'MAIN.agc.bin'),source_edits='LM none; CM existing independently audited indentation-only pseudo-label repairs',containing_file_status='partial',scope='Explicit source SHORTV left branch, VSSL loop counts1..8, OVERFLOW/Y/Z and SETOVF2; excludes right shift and generic instruction processing.',native_sources=['src/sband_shift.c','src/guidance_shift.c'],source_words=sum(len(x['words'])for x in selected),source_line_word_map=selected,constants=constants,entry_bindings={k:sy[k]for k in ['SHORTV','VSSL','DANZIG','OVERFLOW','OVERFLWY','OVERFLWZ','SETOVF2']},coverage='Fresh engine whole-state comparison; no instruction-edge trace or exhaustive Cartesian context claim.')
 (R/'build/sband-shift-source-map.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS exact source shift maps',[(m,x['source_words'])for m,x in result.items()])
if __name__=='__main__':main()
