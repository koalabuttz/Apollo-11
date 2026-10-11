#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Qualify original CM basic trig sources against the pinned full rope."""
import pathlib,subprocess,shutil,hashlib,json,re,difflib
root=pathlib.Path(__file__).resolve().parents[1];baseline=root/'deps/cm-services-fixture';fixture=root/'deps/cm-trig-fixture';fixture.mkdir(exist_ok=True)
for p in baseline.glob('*.agc'):shutil.copyfile(p,fixture/p.name)
audit_sources={}
for name in ['POWERED_FLIGHT_SUBROUTINES.agc','SINGLE_PRECISION_SUBROUTINES.agc','P40-P47.agc']:
 original=(root.parent/'Comanche055'/name).read_text();normalized=re.sub(r'^\t([+-]\d+D?)\t',r' \1\t\t',original,flags=re.M);normalized=re.sub(r'^([+-]\d+D?)\t',r' \1\t',normalized,flags=re.M)
 (fixture/name).write_text(normalized);(fixture/(name+'.original')).write_text(original);(fixture/(name+'.indentation.patch')).write_text(''.join(difflib.unified_diff(original.splitlines(True),normalized.splitlines(True),fromfile='Comanche055/'+name,tofile='fixture/'+name)))
 audit_sources[name]={'original_sha256':hashlib.sha256(original.encode()).hexdigest(),'normalized_sha256':hashlib.sha256(normalized.encode()).hexdigest(),'normalized_lines':sum(a!=b for a,b in zip(original.splitlines(),normalized.splitlines()))}
with(fixture/'assembly.lst').open('w')as f:subprocess.run([str(root/'deps/virtualagc/yaYUL/yaYUL'),'MAIN.agc'],cwd=fixture,stdout=f,stderr=subprocess.STDOUT,check=True)
rope=(fixture/'MAIN.agc.bin').read_bytes();assert rope==(baseline/'MAIN.agc.bin').read_bytes();audit={'rope_sha256':hashlib.sha256(rope).hexdigest(),'original_sources':audit_sources,'erasable_domain':'Pinned-support assignments; translated groups do not use ALTVAR/R53FLBIT.'};assert audit['rope_sha256']=='2ba31de9291cd10fb351a64d261bae8514a1cb75b4651bfa6a135dfa821a2d79'
(root/'build/cm-trig-source-audit.json').write_text(json.dumps(audit,indent=2)+'\n');print(json.dumps(audit,indent=2))
