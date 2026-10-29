#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Qualify byte-original RTB/RT8 sources in original declaration domains."""
import pathlib,shutil,subprocess,hashlib,json
r=pathlib.Path(__file__).resolve().parents[1];records={}
for mission,tree,name,base in [('lm','Luminary099','RTB_OP_CODES.agc','lm-powered-fixture'),('cm','Comanche055','RT8_OP_CODES.agc','cm-powered-fixture')]:
 source=r.parent/tree/name;out=r/f'deps/{mission}-rtb-fixture';out.mkdir(exist_ok=True)
 for p in(r/'deps'/base).glob('*.agc'):shutil.copyfile(p,out/p.name)
 target='RTB_OP_CODES.agc';shutil.copyfile(source,out/target)
 with(out/'assembly.lst').open('w')as f:subprocess.run([str(r/'deps/virtualagc/yaYUL/yaYUL'),'MAIN.agc'],cwd=out,stdout=f,stderr=subprocess.STDOUT,check=True)
 assert(out/'MAIN.agc.bin').read_bytes()==(r/'deps'/base/'MAIN.agc.bin').read_bytes()
 records[mission]=dict(source_file=tree+'/'+name,original_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),assembled_sha256=hashlib.sha256((out/target).read_bytes()).hexdigest(),source_edits=0,assembly_include=target,rope_sha256=hashlib.sha256((out/'MAIN.agc.bin').read_bytes()).hexdigest(),support_source_audit=f'evidence/{mission}-powered-source-audit.json')
(r/'build/rtb-angles-source-audit.json').write_text(json.dumps(records,indent=2)+'\n');print(json.dumps(records,indent=2))
