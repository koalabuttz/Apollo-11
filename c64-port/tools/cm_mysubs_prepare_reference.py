#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Independently assemble original CM MYSUBS with pinned support."""
import pathlib,subprocess,shutil,hashlib,json
r=pathlib.Path(__file__).resolve().parents[1];base=r/'deps/cm-trig-fixture';out=r/'deps/cm-mysubs-fixture';out.mkdir(exist_ok=True)
for p in base.glob('*.agc'):shutil.copyfile(p,out/p.name)
name='MYSUBS.agc';source=r.parent/'Comanche055'/name;shutil.copyfile(source,out/name)
with (out/'assembly.lst').open('w')as f:subprocess.run([str(r/'deps/virtualagc/yaYUL/yaYUL'),'MAIN.agc'],cwd=out,stdout=f,stderr=subprocess.STDOUT,check=True)
assert (out/'MAIN.agc.bin').read_bytes()==(base/'MAIN.agc.bin').read_bytes()
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
x={'original_source':str(source.relative_to(r.parent)),'original_sha256':sha(source),'assembled_source_sha256':sha(out/name),'source_edits':0,'rope_sha256':sha(out/'MAIN.agc.bin'),'pinned_support_fixture':'deps/cm-trig-fixture'}
assert x['rope_sha256']=='2ba31de9291cd10fb351a64d261bae8514a1cb75b4651bfa6a135dfa821a2d79'
(r/'build/cm-mysubs-source-audit.json').write_text(json.dumps(x,indent=2)+'\n');print(json.dumps(x,indent=2))
