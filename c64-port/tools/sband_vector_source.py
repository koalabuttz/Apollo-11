#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source/assembled-word maps of the explicit VXV/VPROJ supporting bodies."""
import hashlib,json,pathlib
from fixed_data_generate import listing_records,symbols,fixed_address,rope_word
R=pathlib.Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 result={}
 for mission,name in [('LM','Luminary099'),('CM','Comanche055')]:
  work=R/'build/sband-source'/name;original=R.parent/name/'INTERPRETER.agc';fixture=work/'INTERPRETER.agc';listing=(work/'assembly.lst').read_text();sy=symbols(listing);rope=(work/'MAIN.agc.bin').read_bytes()
  def address(label):return fixed_address(sy[label])
  intervals=[(address(a),address(b),a+' through before '+b)for a,b in [('VPROJ','MPACVBUF'),('DMPSUB','ROUNDSUB'),('PREDOT','POWRSERS'),('OVERFLWZ','BVSU'),('ENDVXV','DSU'),('VMODE','PUSHUP')]]
  intervals +=[(address('VROUND'),address('PREDOT'),'VROUND/SETOVF2'),(address('NEWMODE'),address('NEWMODE')+1,'NEWMODE')]
  records=listing_records(work/'assembly.lst',{'INTERPRETER.agc':name+'/INTERPRETER.agc'},{name+'/INTERPRETER.agc':original.read_text()},rope)
  selected=[]
  for rec in records:
   at=rec['physical_fixed_word'];groups=[group for low,high,group in intervals if at is not None and low<=at<high]
   if rec['words'] and groups:selected.append(dict(rec,native_groups=groups))
  trace=work.parent.parent/('sband-vector-'+mission+'-reference.trace')
  seen={int(x,8) for x in trace.read_text().splitlines()} if trace.exists() else set()
  mapped={rec['physical_fixed_word']+i for rec in selected for i in range(len(rec['words']))}
  execution=dict(source_words=len(mapped),executed_source_words=len(mapped&seen),unexecuted_source_words=[format(x,'05o') for x in sorted(mapped-seen)],note='PC coverage records actual instruction entries. The source OVERFLOW TC Q no-final-carry exit is compiled/reviewed but not executed by this cohort; no exhaustive Cartesian/all-control-edge claim.')
  constants={k:dict(address=sy[k],physical_fixed_word=address(k),word=rope_word(rope,address(k))) for k in ['ONE','TWO','FOUR','FIVE','LIMITS','POSMAX','NEGMAX'] if k in sy}
  fixed_axes={k:dict(address=sy[k],physical_fixed_word=address(k),words=[rope_word(rope,address(k)+i) for i in range(6)]) for k in ('HIUNITX','HIUNITY','HIUNITZ')}
  assert [fixed_axes[k]['words'] for k in ('HIUNITX','HIUNITY','HIUNITZ')]==[[0o20000,0,0,0,0,0],[0,0,0o20000,0,0,0],[0,0,0,0,0o20000,0]]
  assert [constants[k]['word'] for k in ('ONE','TWO','FOUR','FIVE')]==[1,2,4,5]
  assert [rope_word(rope,address('LIMITS')+i) for i in (-1,0,1)]==[0o37777,0o57777,0o40000]
  result[mission]=dict(source_path=name+'/INTERPRETER.agc',original_sha256=sha(original),assembled_sha256=sha(fixture),rope_sha256=sha(work/'MAIN.agc.bin'),source_edits='LM none; CM previously audited indentation-only pseudo-label repairs',containing_file_status='partial',scope='Explicit VXV/VPROJ, original DMPSUB/DOTSUB/round/overflow controls, ordinary erasable or explicitly supplied immutable fixed operand vectors; no interpreter decoder or arbitrary dispatch.',native_source='src/sband_vector.c',source_words=sum(len(r['words']) for r in selected),source_line_word_map=selected,constants=constants,fixed_axes=fixed_axes,execution=execution,fixed_argument_cases="First256 of each fixed group retain actual original HIUNITX/Y/Z bytes and actual FB. Later cases inject six ordinary input data words into that same passive fixed-data region; interpreter instruction/constant words remain unchanged. The complete baseline rope hash names original data; arbitrary argument injection is separately disclosed.")
 (R/'build/sband-vector-source-map.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS exact source words',[(m,r['source_words'])for m,r in result.items()])
if __name__=='__main__':main()
