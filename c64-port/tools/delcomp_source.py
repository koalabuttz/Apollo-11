#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Reassemble byte-original containing files, map the DELCOMP routine only."""
from pathlib import Path
import gzip,hashlib,json,shutil,subprocess
from fixed_data_generate import listing_records
ROOT=Path(__file__).resolve().parents[1]
def main():
 result=[]
 for mission,name,first,last in [('Luminary099','ATTITUDE_MANEUVER_ROUTINE.agc',828,920),('Comanche055','ANGLFIND.agc',443,535)]:
  work=ROOT/'build/delcomp-source'/mission;work.mkdir(parents=True,exist_ok=True)
  for p in (ROOT/'build/gimbal-source'/mission).glob('*.agc'):shutil.copyfile(p,work/p.name)
  original=ROOT.parent/mission/name;shutil.copyfile(original,work/name)
  assert original.read_bytes()==(work/name).read_bytes()
  with (work/'assembly.lst').open('w') as out:subprocess.run([str(ROOT/'deps/virtualagc/yaYUL/yaYUL'),'MAIN.agc'],cwd=work,stdout=out,stderr=subprocess.STDOUT,check=True)
  rope=(work/'MAIN.agc.bin').read_bytes();assert rope==(ROOT/'build/gimbal-source'/mission/'MAIN.agc.bin').read_bytes()
  path=mission+'/'+name;rows=listing_records(work/'assembly.lst',{name:path},{path:original.read_text()},rope)
  rows=[row for row in rows if first<=row['line']<=last]
  assert rows[0]['label']=='DELCOMP' and rows[-1]['op']=='RVQ',(rows[0],rows[-1])
  for row in rows:row['native_mapping']='src/delcomp.c explicit DELCOMP source sequence or compiled original placement';row['completion']='qualified DELCOMP routine'
  result.append({'source_path':path,'source_sha256':hashlib.sha256(original.read_bytes()).hexdigest(),'original_bytes':True,'indentation_repairs':[],
    'rope_sha256':hashlib.sha256(rope).hexdigest(),'qualified_domain':'DELCOMP entry through its original RVQ return; continuation body external',
    'full_file_status':'partial; other routines in containing file not qualified here','source_line_and_word_map':rows,'instruction_decoder':False})
  (ROOT/f'evidence/delcomp-{mission}-original-assembly.lst.gz').write_bytes(gzip.compress((work/'assembly.lst').read_bytes(),mtime=0))
 (ROOT/'build/delcomp-source/maps.json').write_text(json.dumps(result,indent=2))
 (ROOT/'evidence/delcomp-source-audit.json.gz').write_bytes(gzip.compress(json.dumps({'schema':1,'source_files':result},indent=2).encode(),mtime=0))
 print('PASS: byte-original DELCOMP containing files reassemble with unchanged ropes; precise routine-only source maps')
if __name__=='__main__':main()
