#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Both original programs: five blank source caller/CCSHOLE source states, no native oracle."""
import pathlib,subprocess,hashlib,sys,shutil,re,json
R=pathlib.Path(__file__).resolve().parents[1];E=R/'deps/virtualagc/yaAGC';ropes=[];audit={}
for program,tree,source in [('lm','Luminary099','deps/fixture'),('cm','Comanche055','deps/cm-services-fixture')]:
 S=R/source;F=R/('build/dsky-five-blank-'+program+'-fixture');F.mkdir(exist_ok=True)
 if not(S/'MAIN.agc.bin').exists():subprocess.run(['python3','tools/cm_services_prepare_reference.py' if program=='cm' else 'tools/prepare_reference.py'],cwd=R,check=True)
 for p in S.glob('*.agc'):shutil.copyfile(p,F/p.name)
 metadata={}
 for name in ['PINBALL_GAME_BUTTONS_AND_LIGHTS.agc','T4RUPT_PROGRAM.agc','INTERPRETER.agc','EXECUTIVE.agc','DISPLAY_INTERFACE_ROUTINES.agc','FRESH_START_AND_RESTART.agc','PINBALL_NOUN_TABLES.agc']:
  raw=(R.parent/tree/name).read_bytes();original=raw.decode();normalized=re.sub(r'^[ \t]*([+-]\d+D?)\t',r' \1\t\t',original,flags=re.M)if program=='cm'else original
  target='PINBALL_GAME__BUTTONS_AND_LIGHTS.agc'if program=='cm'and name.startswith('PINBALL_GAME')else name
  (F/(name+'.original')).write_bytes(raw);(F/target).write_text(normalized)
  metadata[name]={'original_sha256':hashlib.sha256(raw).hexdigest(),'normalized_fixture':target,'indentation_repairs':sum(a!=b for a,b in zip(original.splitlines(),normalized.splitlines()))}
 with (F/'assembly.lst').open('w')as out:subprocess.run([str(R/'deps/virtualagc/yaYUL/yaYUL'),'MAIN.agc'],cwd=F,stdout=out,stderr=subprocess.STDOUT,check=True)
 assert(F/'MAIN.agc.bin').read_bytes()==(S/'MAIN.agc.bin').read_bytes()
 metadata['rope_sha256']=hashlib.sha256((F/'MAIN.agc.bin').read_bytes()).hexdigest();audit[program]=metadata;ropes.append(str(F/'MAIN.agc.bin'))
(R/'build/dsky-five-blank-source-audit.json').write_text(json.dumps(audit,indent=2)+'\n')
core=['src/agc_word.c','src/dsky_pinball.c','src/dsky_five_blank.c','tests/p66_bias_cases.c','tests/dsky_five_blank_cases.c']
subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests']+core+['tools/dsky_five_blank_snapshot.c','-o','build/dsky-five-blank-native'],cwd=R,check=True)
subprocess.run(['cc','-std=c99','-O2','-ffunction-sections','-Iinclude','-Itests','-I'+str(E),'src/agc_word.c','tests/p66_bias_cases.c','tests/dsky_five_blank_cases.c','tools/dsky_five_blank_reference.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-Wl,--gc-sections','-o','build/dsky-five-blank-reference'],cwd=R,check=True)
for kind,cmd in [('native',['./build/dsky-five-blank-native']),('reference',['./build/dsky-five-blank-reference']+ropes)]:
 with(R/f'build/dsky-five-blank-{kind}.snapshots').open('w')as out:subprocess.run(cmd,cwd=R,stdout=out,check=True)
errors=[]
with(R/'build/dsky-five-blank-native.snapshots').open()as a,(R/'build/dsky-five-blank-reference.snapshots').open()as b:
 for n,(x,y)in enumerate(zip(a,b)):
  av=list(map(int,x.split()));bv=list(map(int,y.split()));assert len(av)==len(bv)==2053
  if av!=bv and len(errors)<20:errors.append((n,[(oct(i-2),oct(v),oct(w))for i,(v,w)in enumerate(zip(av,bv))if v!=w]))
 assert n==8191
if errors:sys.exit(str(errors))
print('PASS:8192 complete original LM+CM five blank source caller/CCSHOLE states, all2048 words plusSUPER/IRQ/route')
print('Snapshot SHA256:',hashlib.sha256((R/'build/dsky-five-blank-reference.snapshots').read_bytes()).hexdigest())
