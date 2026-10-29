#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Combined original CM Executive/ISR/task/interpreter continuous fixture."""
import pathlib,shutil,subprocess,json,hashlib
R=pathlib.Path(__file__).resolve().parents[1];F=R/'deps/cm-mission-fixture';V=R/'deps/virtualagc';F.mkdir(exist_ok=True)
assert subprocess.check_output(['git','-C',str(V),'rev-parse','HEAD'],text=True).strip()=='1b5d6474841a7ab53237ead070f76f35f162f68d'
for p in (R/'deps/cm-entry-tasks-fixture').glob('*.agc'):shutil.copyfile(p,F/p.name)
sources={}
for name,folder in [('EXECUTIVE.agc','cm-services-fixture'),('WAITLIST.agc','cm-services-fixture'),('INTERRUPT_LEAD_INS.agc','cm-services-fixture'),('INTERPRETER.agc','cm-finish-fixture'),('P40-P47.agc','cm-entry-tasks-fixture'),('CM_ENTRY_DIGITAL_AUTOPILOT.agc','cm-entry-tasks-fixture')]:
 source=R/'deps'/folder;original=(R.parent/'Comanche055'/name).read_bytes();assert (source/(name+'.original')).read_bytes()==original
 for suffix in ['', '.original','.indentation.patch']:shutil.copyfile(source/(name+suffix),F/(name+suffix))
 sources[name]={'original_sha256':hashlib.sha256(original).hexdigest(),'normalized_sha256':hashlib.sha256((F/name).read_bytes()).hexdigest()}
with(F/'assembly.lst').open('w')as out:subprocess.run([str(V/'yaYUL/yaYUL'),'MAIN.agc'],cwd=F,stdout=out,stderr=subprocess.STDOUT,check=True)
rope=(F/'MAIN.agc.bin').read_bytes();assert rope==(R/'deps/cm-entry-tasks-fixture/MAIN.agc.bin').read_bytes()
metadata={'sources':sources,'semantic_repairs':0,'rope_sha256':hashlib.sha256(rope).hexdigest()};(R/'build/cm-mission-source-audit.json').write_text(json.dumps(metadata,indent=2)+'\n');print(json.dumps(metadata,indent=2))
