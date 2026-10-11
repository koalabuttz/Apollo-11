#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import hashlib,json,pathlib
from fixed_data_generate import listing_records,symbols
R=pathlib.Path(__file__).resolve().parents[1]
def main():
 result={};audits=json.loads((R/'build/sband-source/audit.json').read_text())
 for mission,name in [('LM','Luminary099'),('CM','Comanche055')]:
  work=R/'build/sband-source'/name;sy=symbols((work/'assembly.lst').read_text());rope=(work/'MAIN.agc.bin').read_bytes();audit=next(x for x in audits if x['source_path'].startswith(name+'/'));at=0o3745 if mission=='LM'else 0o3750
  pair=[x for x in audit['source_line_word_map']if x['words']and int(x['assembled_address'].split(',')[-1],8)==at];assert len(pair)==1 and pair[0]['words']==[0o77776]
  path=f'{name}/INTERPRETER.agc';original=R.parent/path;rows=listing_records(work/'assembly.lst',{'INTERPRETER.agc':path},{path:original.read_text()},rope)
  newops=int(sy['NEWOPS'].split(',')[-1],8);exitpc=int(sy['EXIT'].split(',')[-1],8);selected=[x for x in rows if x['words']and x['assembled_address'] and (newops<=int(x['assembled_address'].split(',')[-1],8)<newops+0o13 or exitpc<=int(x['assembled_address'].split(',')[-1],8)<exitpc+4)]
  result[mission]=dict(source_path=audit['source_path'],source_sha256=audit['source_sha256'],rope_sha256=audit['rope_sha256'],source_edits=0,source_words=1,source_line_word_map=pair,interpreter_source_path=path,interpreter_source_sha256=hashlib.sha256(original.read_bytes()).hexdigest(),interpreter_controls=selected,bindings={k:sy[k]for k in ['NEWOPS','EXIT','BANKSET','LOC','CYR','EDOP']},endpoint=oct(at+1),domain='Actual original pending antenna EXIT, MODE0 or-1, coherent originalFB/BANKSET64 plusEB, ZERO0/NEWJOB0, ordinaryRAM/Q andraw16A/L, exclusive ownership, all four bank42SUPER forms andbothIRQ. No display/scheduler body.')
 (R/'build/sband-exit-source-map.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS both original EXIT words and compiled original NEWOPS/EXIT control maps')
if __name__=='__main__':main()
