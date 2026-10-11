#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Assemble byte-original CM RCS matrix update and its original aliases."""
import pathlib,shutil,subprocess,hashlib,json,re,difflib
r=pathlib.Path(__file__).resolve().parents[1];base=r/'deps/cm-powered-fixture';out=r/'deps/cm-rcs-matrix-fixture';out.mkdir(exist_ok=True)
for p in base.glob('*.agc'):shutil.copyfile(p,out/p.name)
records={}
for name in ['RCS-CSM_DAP_EXECUTIVE_PROGRAMS.agc','MYSUBS.agc']:
 p=r.parent/'Comanche055'/name;text=p.read_text();normalized=re.sub(r'^\t([+-]\d+D?)\t',r' \1\t\t',text,flags=re.M);normalized=re.sub(r'^([+-]\d+D?)\t',r' \1\t',normalized,flags=re.M);(out/name).write_text(normalized);(out/(name+'.indentation.patch')).write_text(''.join(difflib.unified_diff(text.splitlines(True),normalized.splitlines(True),fromfile='Comanche055/'+name,tofile='fixture/'+name)));records[name]={'original_sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'assembled_source_sha256':hashlib.sha256(normalized.encode()).hexdigest(),'indentation_repairs':[i for i,(a,b)in enumerate(zip(text.splitlines(),normalized.splitlines()),1)if a!=b]}
with(out/'assembly.lst').open('w')as f:subprocess.run([str(r/'deps/virtualagc/yaYUL/yaYUL'),'MAIN.agc'],cwd=out,stdout=f,stderr=subprocess.STDOUT,check=True)
assert(out/'MAIN.agc.bin').read_bytes()==(base/'MAIN.agc.bin').read_bytes()
x={'rope_sha256':hashlib.sha256((out/'MAIN.agc.bin').read_bytes()).hexdigest(),'original_sources':records,'support_source_audit':'evidence/cm-powered-trig-source-audit.json','native_aliases':{'SPSIN1':'SPSIN','SPSIN2':'SPSIN','SPCOS1':'SPCOS','SPCOS2':'SPCOS'},'declaration_data_domain':'Byte-original CM declaration/data baseline'};(r/'build/cm-rcs-matrix-source-audit.json').write_text(json.dumps(x,indent=2)+'\n');print(json.dumps(x,indent=2))
