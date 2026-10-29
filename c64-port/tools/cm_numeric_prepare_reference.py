#!/usr/bin/env python3
"""Original Comanche interpreter overlay, explicit pseudo-label repairs only."""
import pathlib,hashlib,re,difflib,shutil,subprocess,json
R=pathlib.Path(__file__).resolve().parents[1];F=R/'deps/cm-numeric-fixture';S=R/'deps/cm-services-fixture';V=R/'deps/virtualagc'
PIN='1b5d6474841a7ab53237ead070f76f35f162f68d'
assert subprocess.check_output(['git','-C',str(V),'rev-parse','HEAD'],text=True).strip()==PIN
(R/'build').mkdir(exist_ok=True)
if not (S/'MAIN.agc.bin').exists():
 subprocess.run(['python3','tools/cm_services_prepare_reference.py'],cwd=R,check=True)
assert hashlib.sha256((S/'MAIN.agc.bin').read_bytes()).hexdigest()=='2ba31de9291cd10fb351a64d261bae8514a1cb75b4651bfa6a135dfa821a2d79'
F.mkdir(exist_ok=True)
for p in S.glob('*.agc'):shutil.copyfile(p,F/p.name)
metadata={'pin':PIN,'sources':{},'semantic_source_edits':0};modified={}
for name in ['INTERPRETER.agc','POWERED_FLIGHT_SUBROUTINES.agc']:
 p=R.parent/'Comanche055'/name;original=p.read_bytes();text=original.decode()
 normalized=re.sub(r'^\t([+-]\d+D?)\t',r' \1\t\t',text,flags=re.M)
 normalized=re.sub(r'^([+-]\d+D?)\t',r' \1\t',normalized,flags=re.M)
 (F/(name+'.original')).write_bytes(original);(F/name).write_text(normalized);modified[name]=normalized
 (F/(name+'.indentation.patch')).write_text(''.join(difflib.unified_diff(text.splitlines(True),normalized.splitlines(True),fromfile='Comanche055/'+name,tofile='fixture/'+name)))
 metadata['sources'][name]={'original_sha256':hashlib.sha256(original).hexdigest(),'normalized_sha256':hashlib.sha256(normalized.encode()).hexdigest(),'indentation_repairs':sum(a!=b for a,b in zip(text.splitlines(),normalized.splitlines())),'source_original_byte_identical':(F/(name+'.original')).read_bytes()==p.read_bytes()}
with (F/'assembly.lst').open('w') as log:subprocess.run([str(V/'yaYUL/yaYUL'),'MAIN.agc'],cwd=F,stdout=log,stderr=subprocess.STDOUT,check=True)
rope=(F/'MAIN.agc.bin').read_bytes();assert rope==(S/'MAIN.agc.bin').read_bytes()
for name in modified:shutil.copyfile(V/'Comanche055'/name,F/name)
with (F/'pinned-assembly.lst').open('w') as log:subprocess.run([str(V/'yaYUL/yaYUL'),'MAIN.agc'],cwd=F,stdout=log,stderr=subprocess.STDOUT,check=True)
assert rope==(F/'MAIN.agc.bin').read_bytes()
for name,normalized in modified.items():(F/name).write_text(normalized)
metadata['rope_sha256']=hashlib.sha256(rope).hexdigest()
(R/'build/cm-numeric-source-audit.json').write_text(json.dumps(metadata,indent=2)+'\n');print(json.dumps(metadata,indent=2))
print('PASS: original CM interpreter/powered helpers, pseudo-label indentation only; complete rope equals independent pinned assembly and existing CM fixture')
