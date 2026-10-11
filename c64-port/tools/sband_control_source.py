#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Derive compile-time antenna basic caller metadata from original listings."""
import hashlib,json,pathlib
from fixed_data_generate import symbols,fixed_address,rope_word
R=pathlib.Path(__file__).resolve().parents[1]
def main():
 audits=json.loads((R/'build/sband-source/audit.json').read_text());rows=[];out=[]
 for mission,a in enumerate(audits):
  tree=a['source_path'].split('/')[0];work=R/'build/sband-source'/tree;sy=symbols((work/'assembly.lst').read_text());rope=(work/'MAIN.agc.bin').read_bytes();records=a['source_line_word_map']
  def logical(label):return int(sy[label].split(',')[-1],8)
  def constant(label):return rope_word(rope,fixed_address(sy[label]))
  def record(op,arg):return next(x for x in records if x['op']==op and x['arguments']==arg and x['words'])
  start=logical('SBANDANT');display=logical('SBANDEX')+1 if not mission else int(record('CA','EXTVBACT')['assembled_address'].split(',')[-1],8)
  b5=[x for x in records if x['op']=='TC' and x['arguments']=='B5OFF'];term=int(b5[0]['assembled_address'].split(',')[-1],8);proceed=term+1;recycle=term+2;blank=term+3;restart=term+5;reenter=term+(7 if mission else 7)
  if mission:reenter=term+0o10;restart=term+5
  values=[start,display,0 if mission else display+6,term,proceed,recycle,blank,restart,reenter,logical('BANKCALL'),logical('R02BOTH'),record('CADR','R02BOTH')['words'][0],logical('ENDEXT'),logical('GOMARKFR'),record('CADR','GOMARKFR')['words'][0],logical('B5OFF'),logical('ENDOFJOB'),logical('BLANKET'),logical('PRIOCHNG'),logical('DELAYJOB'),record('CADR','DELAYJOB')['words'][0] if mission else 0,logical('INTPRET'),logical('NEWOPS'),constant('PRIO5'),constant('PRIO4'),constant('V06N51')]
  rows.append(' {'+','.join('0'+format(v,'o') for v in values)+'},')
  selected=[r for r in records if r['words'] and (r['line']<=record('TC','INTPRET')['line'] or r['line']>=record('CA','EXTVBACT')['line'])]
  out.append(dict(source_path=a['source_path'],source_sha256=a['source_sha256'],rope_sha256=a['rope_sha256'],scope='Basic antenna entry/INTPRET front and outer display caller controls. Interpreted arithmetic body remains pending native composition; external callee bodies not claimed.',containing_file_status='partial',source_line_word_map=selected,bindings=values))
 names=['start','display','after_priority','terminate','proceed','recycle','blank','restart','reenter','bankcall','r02both','r02cadr','endext','gomarkfr','gomarkcadr','b5off','endofjob','blanket','priochng','delayjob','delaycadr','intpret','newops','prio5','prio4','vn']
 (R/'include/sband_control_source.h').write_text('/* SPDX-License-Identifier: MIT; compile-time original source metadata. */\n#ifndef SBAND_CONTROL_SOURCE_H\n#define SBAND_CONTROL_SOURCE_H\n#include <stdint.h>\ntypedef struct {uint16_t '+','.join(names)+';} sband_control_bindings;\nstatic const sband_control_bindings sband_control_source[2]={\n'+'\n'.join(rows)+'\n};\n#endif\n')
 (R/'build/sband-control-source-audit.json').write_text(json.dumps(out,indent=2)+'\n');print('PASS original antenna basic caller metadata',rows)
if __name__=='__main__':main()
