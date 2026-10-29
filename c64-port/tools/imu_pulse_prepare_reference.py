#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Original IMU service fixture, isolated from frozen RTB source fixtures."""
import pathlib,subprocess,shutil,hashlib,json,re,difflib
r=pathlib.Path(__file__).resolve().parents[1];records={}
for mission,tree in [('lm','Luminary099'),('cm','Comanche055')]:
 base=r/f'deps/{mission}-rtb-fixture';out=r/f'deps/{mission}-imu-pulse-fixture';out.mkdir(exist_ok=True)
 for p in base.glob('*.agc'):shutil.copyfile(p,out/p.name)
 name='IMU_MODE_SWITCHING_ROUTINES.agc';source=r.parent/tree/name;text=source.read_text();normalized=text
 if mission=='cm':
  normalized=re.sub(r'^\t([+-]\d+D?)\t',r' \1\t\t',normalized,flags=re.M);normalized=re.sub(r'^([+-]\d+D?)\t',r' \1\t',normalized,flags=re.M)
 (out/name).write_text(normalized);patch=''.join(difflib.unified_diff(text.splitlines(True),normalized.splitlines(True),fromfile=tree+'/'+name,tofile='fixture/'+name));(out/(name+'.indentation.patch')).write_text(patch)
 with(out/'assembly.lst').open('w')as f:subprocess.run([str(r/'deps/virtualagc/yaYUL/yaYUL'),'MAIN.agc'],cwd=out,stdout=f,stderr=subprocess.STDOUT,check=True)
 assert(out/'MAIN.agc.bin').read_bytes()==(base/'MAIN.agc.bin').read_bytes()
 records[mission]=dict(source_file=tree+'/'+name,original_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),assembled_sha256=hashlib.sha256((out/name).read_bytes()).hexdigest(),indentation_repairs=[i for i,(a,b)in enumerate(zip(text.splitlines(),normalized.splitlines()),1)if a!=b],semantic_edits=0,rope_sha256=hashlib.sha256((out/'MAIN.agc.bin').read_bytes()).hexdigest(),support_source_audit='evidence/rtb-angles-source-audit.json',indentation_patch=patch)
(r/'build/imu-pulse-source-audit.json').write_text(json.dumps(records,indent=2)+'\n');print(json.dumps(records,indent=2))
