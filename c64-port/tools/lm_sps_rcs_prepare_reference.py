#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Assemble byte-original LM backup RCS source in the original data domain."""
import pathlib,shutil,subprocess,hashlib,json
r=pathlib.Path(__file__).resolve().parents[1];base=r/'deps/lm-powered-fixture';out=r/'deps/lm-sps-rcs-fixture';out.mkdir(exist_ok=True)
for p in base.glob('*.agc'):shutil.copyfile(p,out/p.name)
name='SPS_BACK-UP_RCS_CONTROL.agc';source=r.parent/'Luminary099'/name;shutil.copyfile(source,out/name)
with(out/'assembly.lst').open('w')as f:subprocess.run([str(r/'deps/virtualagc/yaYUL/yaYUL'),'MAIN.agc'],cwd=out,stdout=f,stderr=subprocess.STDOUT,check=True)
assert(out/'MAIN.agc.bin').read_bytes()==(base/'MAIN.agc.bin').read_bytes();x={'rope_sha256':hashlib.sha256((out/'MAIN.agc.bin').read_bytes()).hexdigest(),'source_file':'Luminary099/'+name,'original_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'assembled_sha256':hashlib.sha256((out/name).read_bytes()).hexdigest(),'source_edits':0,'support_source_audit':'evidence/lm-powered-source-audit.json','declaration_data_domain':'Byte-original LM declaration/data baseline'};(r/'build/lm-sps-rcs-source-audit.json').write_text(json.dumps(x,indent=2)+'\n');print(json.dumps(x,indent=2))
