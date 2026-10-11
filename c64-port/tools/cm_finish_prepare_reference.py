#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Original CM interpreter control overlay for full Executive dispatch proof."""
import pathlib,shutil,subprocess,re,difflib,json,hashlib
R=pathlib.Path(__file__).resolve().parents[1];S=R/'deps/cm-services-fixture';F=R/'deps/cm-finish-fixture';V=R/'deps/virtualagc';F.mkdir(exist_ok=True)
assert subprocess.check_output(['git','-C',str(V),'rev-parse','HEAD'],text=True).strip()=='1b5d6474841a7ab53237ead070f76f35f162f68d'
for p in S.glob('*.agc'):shutil.copyfile(p,F/p.name)
name='INTERPRETER.agc';a=(R.parent/'Comanche055'/name).read_bytes();text=a.decode();b=re.sub(r'^\t([+-]\d+D?)\t',r' \1\t\t',text,flags=re.M);b=re.sub(r'^([+-]\d+D?)\t',r' \1\t',b,flags=re.M)
(F/(name+'.original')).write_bytes(a);(F/name).write_text(b);(F/(name+'.indentation.patch')).write_text(''.join(difflib.unified_diff(text.splitlines(True),b.splitlines(True),fromfile='original/'+name,tofile='fixture/'+name)))
with(F/'assembly.lst').open('w') as o:subprocess.run([str(V/'yaYUL/yaYUL'),'MAIN.agc'],cwd=F,stdout=o,stderr=subprocess.STDOUT,check=True)
assert (F/'MAIN.agc.bin').read_bytes()==(S/'MAIN.agc.bin').read_bytes()
metadata={'source':name,'original_sha256':hashlib.sha256(a).hexdigest(),'normalized_sha256':hashlib.sha256(b.encode()).hexdigest(),'indentation_repairs':sum(x!=y for x,y in zip(text.splitlines(),b.splitlines())),'semantic_repairs':0,'rope_sha256':hashlib.sha256((F/'MAIN.agc.bin').read_bytes()).hexdigest()}
(R/'build/cm-finish-source-audit.json').write_text(json.dumps(metadata,indent=2)+'\n');print(json.dumps(metadata,indent=2))
