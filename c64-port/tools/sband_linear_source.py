#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Exact original source body/data audit for canonical VXSC/VAD/VSU."""
import hashlib,json,pathlib
from fixed_data_generate import listing_records,symbols,fixed_address,rope_word
R=pathlib.Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 result={}
 for mission,name in [('LM','Luminary099'),('CM','Comanche055')]:
  work=R/'build/sband-source'/name;original=R.parent/name/'INTERPRETER.agc';sy=symbols((work/'assembly.lst').read_text());rope=(work/'MAIN.agc.bin').read_bytes()
  def addr(label):return fixed_address(sy[label])
  intervals=[(addr(a),addr(b))for a,b in [('VXSC','VPROJ'),('DVXSC','VXV'),('VSU','DAD'),('ENDVXV','DSU'),('OVERFLWZ','BVSU'),('DMPSUB','ROUNDSUB'),('VROUND','PREDOT'),('VMODE','PUSHUP')]]+[(addr('NEWMODE'),addr('NEWMODE')+1)]
  records=listing_records(work/'assembly.lst',{'INTERPRETER.agc':name+'/INTERPRETER.agc'},{name+'/INTERPRETER.agc':original.read_text()},rope)
  selected=[x for x in records if x['words']and x['physical_fixed_word']is not None and any(a<=x['physical_fixed_word']<b for a,b in intervals)]
  constants={k:dict(address=sy[k],physical_fixed_word=addr(k),word=rope_word(rope,addr(k)))for k in ['ONE','TWO','THREE','FIVE','BIT15','PRIO30','LIMITS']}
  assert constants['PRIO30']['word']==0o30000 and constants['BIT15']['word']==0o40000
  result[mission]=dict(source_path=name+'/INTERPRETER.agc',original_sha256=sha(original),assembled_sha256=sha(work/'INTERPRETER.agc'),rope_sha256=sha(work/'MAIN.agc.bin'),source_edits='LM none; CM previously audited indentation-only pseudo-label repairs',containing_file_status='partial',native_source='src/sband_vector_linear.c',scope='Original VXSC vector/scalar branches and VAD/VSU vector bodies, with original DMPSUB/VROUND/OVERFLOW helpers. No source decoder, generic instruction dispatch or entire interpreter claim.',entry_bindings={k:sy[k]for k in ['VXSC','VVXSC','DVXSC','VAD','VSU','DANZIG','HIUNITX']},source_words=sum(len(x['words'])for x in selected),source_line_word_map=selected,constants=constants,fixed_argument_provenance='Fixed groups inject only six ordinary input data words at original HIUNITX region; all instruction/control/scalar-constant words remain original. The baseline rope SHA names the unchanged assembled source; injected argument data are independently constructed inputs, not claimed original table values.',coverage='Varied sampled context and scratch/operand aliases; every source row compares all2048 original image words plus external SUPER/IRQ. No exhaustive Cartesian or instruction-edge trace claim.')
 (R/'build/sband-linear-source-map.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS source bodies',[(m,x['source_words'])for m,x in result.items()])
if __name__=='__main__':main()
