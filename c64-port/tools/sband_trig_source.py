#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Audit canonical trig entries and the unchanged compiled numerical literals."""
import hashlib,json,pathlib,re
from fixed_data_generate import listing_records,symbols,fixed_address,rope_word
R=pathlib.Path(__file__).resolve().parents[1]
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 result={};native=(R/'src/guidance_trig.c').read_text()
 literal=re.search(r'coefficient\[5\]=\{(.*?)\};',native)[1]
 coefficients=[int(x,8) for x in re.findall(r'0[0-7]+',literal)]
 assert len(coefficients)==10
 for mission,name in [('LM','Luminary099'),('CM','Comanche055')]:
  work=R/'build/sband-source'/name;original=R.parent/name/'INTERPRETER.agc';listing=(work/'assembly.lst').read_text();sy=symbols(listing);rope=(work/'MAIN.agc.bin').read_bytes()
  def address(label):return fixed_address(sy[label])
  records=listing_records(work/'assembly.lst',{'INTERPRETER.agc':name+'/INTERPRETER.agc'},{name+'/INTERPRETER.agc':original.read_text()},rope)
  selected=[r for r in records if r['words'] and r['physical_fixed_word'] is not None and address('COSINE')<=r['physical_fixed_word']<address('ARCSIN')]
  poly=next(r for r in selected if r['op']=='TC' and r['arguments']=='POLY')['physical_fixed_word']
  assert rope_word(rope,poly+1)==3
  assembled=[rope_word(rope,poly+2+i)for i in range(10)]
  assert assembled==coefficients
  constants={k:dict(address=sy[k],physical_fixed_word=address(k),word=rope_word(rope,address(k)))for k in ['QUARTER','NEG1/2','ONE','LBUF2']}
  assert constants['QUARTER']['word']==0o10000
  trace=R/f'build/sband-trig-{mission}-reference.trace';seen={int(x,8)for x in trace.read_text().splitlines()}
  code={int(r['assembled_address'].split(',')[-1],8)+i for r in selected if r['op'] not in ('DEC','2DEC') for i in range(len(r['words']))}
  result[mission]=dict(source_path=name+'/INTERPRETER.agc',original_sha256=digest(original),assembled_sha256=digest(work/'INTERPRETER.agc'),rope_sha256=digest(work/'MAIN.agc.bin'),scope='Canonical DP MODE0 SINE/COSINE entry through pending DANZIG, using frozen compiled DSQSUB/POLY/DMPSUB arithmetic; no decoder or whole INTERPRETER claim.',containing_file_status='partial',source_edits='LM none; CM existing audited indentation-only repairs',native_sources=['src/sband_trig.c','src/guidance_trig.c','src/guidance_product.c'],entry_bindings={k:sy[k]for k in ['SINE','COSINE','DANZIG','DSQSUB','POLY','DMPSUB']},source_line_word_map=selected,original_coefficient_words=assembled,constants=constants,execution=dict(code_words=len(code),executed_code_words=len(code&seen),unexecuted_code_words=[format(x,'05o')for x in sorted(code-seen)]),degree=3)
 (R/'build/sband-trig-source-map.json').write_text(json.dumps(result,indent=2)+'\n')
 print('PASS source entry/data literals',[(m,len(r['source_line_word_map']),r['execution'])for m,r in result.items()])
if __name__=='__main__':main()
