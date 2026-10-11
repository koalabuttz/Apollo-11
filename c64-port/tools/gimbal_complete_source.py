#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-exact whole gimbal files with disclosed original helper support."""
from pathlib import Path
import difflib,gzip,hashlib,json,re,shutil,subprocess
from fixed_data_generate import listing_records,source_line
ROOT=Path(__file__).resolve().parents[1]
def main():
 result=[]
 for mission in ['Luminary099','Comanche055']:
  work=ROOT/'build/gimbal-complete-source'/mission;work.mkdir(parents=True,exist_ok=True);support=[]
  for p in (ROOT/'build/delcomp-source'/mission).glob('*.agc'):shutil.copyfile(p,work/p.name)
  for name in ['INTERPRETER.agc','RTB_OP_CODES.agc' if mission=='Luminary099' else 'RT8_OP_CODES.agc']:
   original=ROOT.parent/mission/name;text=original.read_text();normalized=text
   if mission=='Comanche055':
    normalized=re.sub(r'^\t([+-]\d+D?)\t',r' \1\t\t',normalized,flags=re.M);normalized=re.sub(r'^([+-]\d+D?)\t',r' \1\t',normalized,flags=re.M)
   (work/name).write_text(normalized)
   changes=[i for i,(a,b) in enumerate(zip(text.splitlines(),normalized.splitlines()),1) if a!=b]
   if mission=='Luminary099':assert not changes
   else:assert len(changes)==(36 if name=='INTERPRETER.agc' else 0)
   patch=''.join(difflib.unified_diff(text.splitlines(True),normalized.splitlines(True),fromfile=mission+'/'+name,tofile='fixture/'+name))
   (ROOT/f'evidence/gimbal-complete-{mission}-{name}.indentation.patch').write_text(patch)
   support.append({'source_path':mission+'/'+name,'source_sha256':hashlib.sha256(original.read_bytes()).hexdigest(),'normalized_sha256':hashlib.sha256(normalized.encode()).hexdigest(),'indentation_repair_lines':changes,'numerical_source_edits':0})
  with (work/'assembly.lst').open('w') as out:subprocess.run([str(ROOT/'deps/virtualagc/yaYUL/yaYUL'),'MAIN.agc'],cwd=work,stdout=out,stderr=subprocess.STDOUT,check=True)
  rope=(work/'MAIN.agc.bin').read_bytes();assert rope==(ROOT/'build/delcomp-source'/mission/'MAIN.agc.bin').read_bytes()
  original=ROOT.parent/mission/'GIMBAL_LOCK_AVOIDANCE.agc';name=mission+'/'+original.name
  assert original.read_bytes()==(work/original.name).read_bytes()
  rows=listing_records(work/'assembly.lst',{original.name:name},{name:original.read_text()},rope)
  assert {r['line'] for r in rows}=={i for i,line in enumerate(original.read_text().splitlines(),1) if source_line(line) is not None}
  called=False
  for row in rows:
   if not row['words']:mapping='compile-time original fixed placement, EBANK declaration or entry alias'
   elif row['op']=='2DEC':mapping='complete immutable src/gimbal_data.inc; gimbal_fixed_read original physical address'
   elif not called:mapping='src/gimbal_stage.c original entry; src/delcomp.c complete external DELCOMP dependency'
   else:mapping='src/gimbal_complete.c selected ARATE load, scalar/vector and fixed QUADROT arithmetic, BRATE/TM/BIASTEMP stores and source SETGO return'
   row['native_mapping']=mapping;row['completion']='complete composed gimbal file'
   if row['op']=='DELCOMP':called=True
  assert called
  helper_name=mission+'/INTERPRETER.agc';helper_original=(ROOT.parent/helper_name).read_text()
  helpers=listing_records(work/'assembly.lst',{'INTERPRETER.agc':helper_name},{helper_name:helper_original},rope)
  # Retain original scalar/vector product, matrix-vector and double-square
  # ranges as dependencies; this does not certify the entire interpreter.
  ranges=[(1140,1252),(2449,2463)] if mission=='Luminary099' else [(1146,1258),(2457,2471)]
  helpers=[r for r in helpers if any(a<=r['line']<=b for a,b in ranges)]
  result.append({'source_path':name,'source_sha256':hashlib.sha256(original.read_bytes()).hexdigest(),'original_bytes':True,'indentation_repairs':[],
   'rope_sha256':hashlib.sha256(rope).hexdigest(),'full_file_status':'complete composed native gimbal file','source_line_and_word_map':rows,'original_helper_support':support,'helper_original_word_maps':helpers,'external_continuation':'NEWANGL+1 body is outside gimbal file','instruction_decoder':False})
  (ROOT/f'evidence/gimbal-complete-{mission}-original-assembly.lst.gz').write_bytes(gzip.compress((work/'assembly.lst').read_bytes(),mtime=0))
 (ROOT/'build/gimbal-complete-source/maps.json').write_text(json.dumps(result,indent=2))
 (ROOT/'evidence/gimbal-complete-source-audit.json.gz').write_bytes(gzip.compress(json.dumps({'schema':1,'source_files':result},indent=2).encode(),mtime=0))
 print('PASS: original complete gimbal files/helper routines assemble; unchanged original-layout ropes, disclosed36 CM indentation-only repairs')
if __name__=='__main__':main()
