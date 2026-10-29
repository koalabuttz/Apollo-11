#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Exact original DOT body and immutable scalar data provenance."""
import hashlib,json,pathlib
from fixed_data_generate import listing_records,symbols,fixed_address,rope_word
R=pathlib.Path(__file__).resolve().parents[1]
def main():
 result={}
 for mission,name in [('LM','Luminary099'),('CM','Comanche055')]:
  work=R/'build/sband-source'/name;original=R.parent/name/'INTERPRETER.agc';sy=symbols((work/'assembly.lst').read_text());rope=(work/'MAIN.agc.bin').read_bytes()
  def addr(label):return fixed_address(sy[label])
  intervals=[(addr(a),addr(b))for a,b in [('DAD','OVERFLWZ'),('OVERFLOW','BVSU'),('DMPSUB','ROUNDSUB'),('DMP1','DMPR')]]+[(addr('SETOVF2'),addr('SETOVF2')+2)]
  records=listing_records(work/'assembly.lst',{'INTERPRETER.agc':name+'/INTERPRETER.agc'},{name+'/INTERPRETER.agc':original.read_text()},rope)
  selected=[x for x in records if x['words']and x['physical_fixed_word']is not None and any(a<=x['physical_fixed_word']<b for a,b in intervals)]
  constants={k:dict(address=sy[k],physical_fixed_word=addr(k),word=rope_word(rope,addr(k)))for k in ['ZERO','LIMITS']}
  assert constants['ZERO']['word']==0
  constants['LIMITS_neighbors']=dict(physical_fixed_word=addr('LIMITS')-1,words=[rope_word(rope,addr('LIMITS')+i)for i in [-1,0,1]],role='OVERFLOW selects neighbor-1/+1 then complements; center passive word unused by this entry')
  assert constants['LIMITS_neighbors']['words']==[0o37777,0o57777,0o40000]
  result[mission]=dict(source_path=name+'/INTERPRETER.agc',original_sha256=hashlib.sha256(original.read_bytes()).hexdigest(),assembled_sha256=hashlib.sha256((work/'INTERPRETER.agc').read_bytes()).hexdigest(),rope_sha256=hashlib.sha256(rope).hexdigest(),source_edits='LM none; CM previously audited indentation-only pseudo-label repairs',containing_file_status='partial',native_source='src/sband_scalar.c',source_words=sum(len(x['words'])for x in selected),source_line_word_map=selected,constants=constants,entry_bindings={k:sy[k]for k in ['DMP1','DAD','DSU','DMPSUB','DANZIG','HIUNITX']},scope='Actual scalar DMP1/DAD/DSU through complete DMPSUB/OVERFLOW numerical controls to pending mission DANZIG. No opcode decoder or whole INTERPRETER/antenna claim.',fixed_argument_provenance='Fixed groups inject only two ordinary input data words at original HIUNITX passive table region; instruction/control/scalar-constant words stay original. Baseline rope identity and supplied input data are distinct.',coverage='Every65536 raw A/L and all32768 ordinary incoming Q values occur per group. Fixed component ordinary marginals retained in mirrored second half. Varied scratch/ERA aliases, EB, all8SUPER latch forms and both IRQ. No Cartesian or allinstruction-edge execution claim.')
 (R/'build/sband-scalar-source-map.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS DP scalar source maps',[(m,x['source_words'])for m,x in result.items()])
if __name__=='__main__':main()
