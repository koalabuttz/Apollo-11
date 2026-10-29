#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import hashlib,json,pathlib,re
from fixed_data_generate import listing_records,symbols,fixed_address,rope_word,physical_erasable
R=pathlib.Path(__file__).resolve().parents[1]
def main():
 result={};audits=json.loads((R/'build/sband-source/audit.json').read_text());native=(R/'src/sband_moon.c').read_text();literal=[int(x,8)for x in re.findall(r'0[0-7]+',re.search(r'remdist\[6\]=\{(.*?)\}',native)[1])]
 for mission,name in [('LM','Luminary099'),('CM','Comanche055')]:
  work=R/'build/sband-source'/name;sy=symbols((work/'assembly.lst').read_text());a=next(x for x in audits if x['source_path'].startswith(name+'/'));ranges=[(0o3627,0o3636),(0o3640,0o3642)]if mission=='LM'else[(0o3673,0o3677),(0o3701,0o3704)]
  selected=[r for r in a['source_line_word_map']if r['words']and any(lo<=int(r['assembled_address'].split(',')[-1],8)<hi for lo,hi in ranges)];constant={}
  if mission=='LM':
   rope=(work/'MAIN.agc.bin').read_bytes();at=fixed_address(sy['REMDIST']);assert [rope_word(rope,at+i)for i in range(6)]==literal;assert physical_erasable(sy['VMOON'])==0o2722 and physical_erasable(sy['RLM'])==0o2204
   original=R.parent/name/'CONTROLLED_CONSTANTS.agc';records=listing_records(work/'assembly.lst',{'CONTROLLED_CONSTANTS.agc':name+'/CONTROLLED_CONSTANTS.agc'},{name+'/CONTROLLED_CONSTANTS.agc':original.read_text()},rope);literal_map=[r for r in records if r['words']and r['physical_fixed_word']==at];assert len(literal_map)==1 and literal_map[0]['words']==literal[:2]
   constant=dict(source_path=name+'/CONTROLLED_CONSTANTS.agc',original_sha256=hashlib.sha256(original.read_bytes()).hexdigest(),assembled_sha256=hashlib.sha256((work/'CONTROLLED_CONSTANTS.agc').read_bytes()).hexdigest(),source_line_word_map=literal_map,address=sy['REMDIST'],words=literal[:2],unused_preflight_padding=literal[2:],padding_scope='Exact four source instruction words immediately following REMDIST are passive unused padding required by six-word kernel preflight, never decoded/executed/credited as algorithm translation.')
  result[mission]=dict(source_path=a['source_path'],source_sha256=a['source_sha256'],rope_sha256=a['rope_sha256'],source_edits=0,containing_file_status='partial',native_source='src/sband_moon.c',source_line_word_map=selected,scope='Actual returned-LUNPOS caller through native Moon-return vector arithmetic/GOTO/SETPD into LM UNIT or CM MXV before helper bodies.',constants=constant,entry_bindings={k:sy[k]for k in ['VMOON','UNIT','MXV','NEWOPS','DANZIG','RLM'if mission=='LM'else 'RCM']},unmapped_remaining='LUNPOS callee numerical body, following body-frame and pitch/yaw computations and display lifecycle.')
 (R/'build/sband-moon-source-map.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS Moon-return source maps',[(m,sum(len(r['words'])for r in x['source_line_word_map']))for m,x in result.items()])
if __name__=='__main__':main()
