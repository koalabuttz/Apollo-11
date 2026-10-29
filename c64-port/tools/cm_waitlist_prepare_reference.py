#!/usr/bin/env python3
"""Assemble original CM queue/interrupt source with recorded layout repairs."""
import difflib,hashlib,json,pathlib,re,shutil,subprocess
ROOT=pathlib.Path(__file__).resolve().parents[1]
PIN='1b5d6474841a7ab53237ead070f76f35f162f68d'
V=ROOT/'deps/virtualagc'; F=ROOT/'deps/cm-waitlist-fixture'
assert subprocess.check_output(['git','-C',str(V),'rev-parse','HEAD'],text=True).strip()==PIN
F.mkdir(parents=True,exist_ok=True)
for p in (V/'Comanche055').glob('*.agc'): shutil.copyfile(p,F/p.name)
metadata={'pin':PIN,'sources':{}}
patched={}
for name in ['EXECUTIVE.agc','WAITLIST.agc','INTERRUPT_LEAD_INS.agc']:
 original=(ROOT.parent/'Comanche055'/name).read_bytes(); text=original.decode()
 normalized=re.sub(r'^\t([+-]\d+)\t',r' \1\t\t',text,flags=re.M)
 normalized=re.sub(r'^([+-]\d+)\t',r' \1\t',normalized,flags=re.M)
 (F/(name+'.original')).write_bytes(original)
 (F/name).write_text(normalized); patched[name]=normalized
 (F/(name+'.indentation.patch')).write_text(''.join(difflib.unified_diff(text.splitlines(True),normalized.splitlines(True),fromfile='Comanche055/'+name,tofile='fixture/'+name)))
 metadata['sources'][name]={'original_sha256':hashlib.sha256(original).hexdigest(),'normalized_sha256':hashlib.sha256(normalized.encode()).hexdigest(),'normalized_lines':sum(a!=b for a,b in zip(text.splitlines(),normalized.splitlines()))}
def assemble(log):
 with (F/log).open('w') as f: subprocess.run([str(V/'yaYUL/yaYUL'),'MAIN.agc'],cwd=F,stdout=f,stderr=subprocess.STDOUT,check=True)
assemble('assembly.lst'); rope=(F/'MAIN.agc.bin').read_bytes()
listing=(F/'assembly.lst').read_text()
for name,addr in [('WAITLIST','5245'),('LVWTLIST','5263'),('SVCT3','5306'),('TASKOVER','5324'),('T3RUPT','01,3416'),('T3RUPT2','01,3423'),('BAILOUT','7755'),('POODOO','5721'),('WAITEXIT','0061'),('WAITBANK','0062'),('WAITADR','0063'),('WAITTEMP','0064')]:
 assert re.search(r'(?<!\w)'+name+r'\s+'+addr+r'\b',listing),name
for name in patched: shutil.copyfile(V/'Comanche055'/name,F/name)
assemble('pinned-assembly.lst')
assert rope==(F/'MAIN.agc.bin').read_bytes(),'complete normalized/pinned ropes differ'
for name,normalized in patched.items():
 (F/name).write_text(normalized)
 assert (F/(name+'.original')).read_bytes()==(ROOT.parent/'Comanche055'/name).read_bytes()
metadata['rope_sha256']=hashlib.sha256(rope).hexdigest()
assert metadata['rope_sha256']=='2ba31de9291cd10fb351a64d261bae8514a1cb75b4651bfa6a135dfa821a2d79'
(ROOT/'build/cm-waitlist-source-audit.json').write_text(json.dumps(metadata,indent=2)+'\n')
print('PASS: original Comanche queue/interrupt overlays; complete rope equals independently assembled pinned source')
print(json.dumps(metadata,indent=2))
