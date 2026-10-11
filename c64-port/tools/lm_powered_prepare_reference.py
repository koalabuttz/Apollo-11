#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Qualify original LM powered source in original declaration/data domain."""
import pathlib,subprocess,shutil,hashlib,json
r=pathlib.Path(__file__).resolve().parents[1];base=r/'build/fixed-data/Luminary099';out=r/'deps/lm-powered-fixture';out.mkdir(exist_ok=True)
for p in base.glob('*.agc'):shutil.copyfile(p,out/p.name)
records={}
for name in ['POWERED_FLIGHT_SUBROUTINES.agc','INTERPRETER.agc','RTB_OP_CODES.agc','SINGLE_PRECISION_SUBROUTINES.agc']:
 source=r.parent/'Luminary099'/name;shutil.copyfile(source,out/name);records[name]={'original_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'assembled_source_sha256':hashlib.sha256((out/name).read_bytes()).hexdigest(),'source_edits':0}
with(out/'assembly.lst').open('w')as f:subprocess.run([str(r/'deps/virtualagc/yaYUL/yaYUL'),'MAIN.agc'],cwd=out,stdout=f,stderr=subprocess.STDOUT,check=True)
assert (out/'MAIN.agc.bin').read_bytes()==(base/'MAIN.agc.bin').read_bytes()
x={'rope_sha256':hashlib.sha256((out/'MAIN.agc.bin').read_bytes()).hexdigest(),'original_sources':records,'declaration_data_domain':'Byte-original LM ERASABLE_ASSIGNMENTS and fixed-data source baseline','historical_source_edits':0}
(r/'build/lm-powered-source-audit.json').write_text(json.dumps(x,indent=2)+'\n');print(json.dumps(x,indent=2))
