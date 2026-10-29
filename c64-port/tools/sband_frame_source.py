#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import json,pathlib
from fixed_data_generate import symbols
R=pathlib.Path(__file__).resolve().parents[1]
def main():
 result={};audits=json.loads((R/'build/sband-source/audit.json').read_text())
 for mission,name in [('LM','Luminary099'),('CM','Comanche055')]:
  sy=symbols((R/'build/sband-source'/name/'assembly.lst').read_text());a=next(x for x in audits if x['source_path'].startswith(name+'/'));ranges=[(0o3640,0o3646)]if mission=='LM'else[(0o3702,0o3705)]
  selected=[r for r in a['source_line_word_map']if r['words']and any(low<=int(r['assembled_address'].split(',')[-1],8)<high for low,high in ranges)]
  assert sy['REFSMMAT']==('E3,1733'if mission=='LM'else 'E3,1735')
  result[mission]=dict(source_path=a['source_path'],source_sha256=a['source_sha256'],rope_sha256=a['rope_sha256'],source_edits=0,containing_file_status='partial',native_source='src/sband_frame.c',source_line_word_map=selected,entry_bindings={k:sy[k]for k in ['UNIT','CDUTRIG','MXV','REFSMMAT','NEWOPS','DANZIG','SHORTV']},scope='LM actual UNIT; LM VCOMP/CALL plus complete original CDUTRIG and genuine RVQ; both actual eighteen-word REFSMMAT MXV followed by original VSL1 selection. Helpers are separate qualified compiled native APIs; no arbitrary decoded dispatch.',external_continuations=['Actual UNIT 1302 alarm frontier, before handler','Pending actual DANZIG, before next source pair or Executive check'],unmapped_remaining='Moon-return vector math, following pushes/body transformations and pitch/yaw calculations, and display lifecycle.')
 (R/'build/sband-frame-source-map.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS antenna frame source maps',[(m,sum(len(x['words'])for x in a['source_line_word_map']))for m,a in result.items()])
if __name__=='__main__':main()
