#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Qualify CM powered-flight source with byte-original declarations/data."""
import pathlib,subprocess,shutil,hashlib,json,re,difflib
r=pathlib.Path(__file__).resolve().parents[1];base=r/'build/fixed-data/Comanche055';out=r/'deps/cm-powered-fixture';out.mkdir(exist_ok=True)
for p in base.glob('*.agc'):shutil.copyfile(p,out/p.name)
records={}
for name in ['POWERED_FLIGHT_SUBROUTINES.agc','INTERPRETER.agc','RT8_OP_CODES.agc','SINGLE_PRECISION_SUBROUTINES.agc']:
 source=r.parent/'Comanche055'/name;text=source.read_text();normalized=re.sub(r'^\t([+-]\d+D?)\t',r' \1\t\t',text,flags=re.M);normalized=re.sub(r'^([+-]\d+D?)\t',r' \1\t',normalized,flags=re.M);(out/name).write_text(normalized);(out/(name+'.original')).write_text(text);(out/(name+'.indentation.patch')).write_text(''.join(difflib.unified_diff(text.splitlines(True),normalized.splitlines(True),fromfile='Comanche055/'+name,tofile='fixture/'+name)))
 records[name]={'original_sha256':hashlib.sha256(text.encode()).hexdigest(),'normalized_sha256':hashlib.sha256(normalized.encode()).hexdigest(),'indentation_repair_lines':[i for i,(a,b)in enumerate(zip(text.splitlines(),normalized.splitlines()),1)if a!=b]}
with(out/'assembly.lst').open('w')as f:subprocess.run([str(r/'deps/virtualagc/yaYUL/yaYUL'),'MAIN.agc'],cwd=out,stdout=f,stderr=subprocess.STDOUT,check=True)
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();rope=sha(out/'MAIN.agc.bin');assert rope=='6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061';assert (out/'MAIN.agc.bin').read_bytes()==(base/'MAIN.agc.bin').read_bytes()
x={'rope_sha256':rope,'original_sources':records,'declaration_data_domain':'Byte-original CM ERASABLE_ASSIGNMENTS and fixed data; ALTVAR01357/R53FLBITBIT8 preserved','declaration_dependency':'docs/FIXED_DATA.md','historical_source_edits':0};(r/'build/cm-powered-source-audit.json').write_text(json.dumps(x,indent=2)+'\n');print(json.dumps(x,indent=2))
