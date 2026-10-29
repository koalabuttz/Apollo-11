#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Full canonical Comanche relay-write original comparison."""
import pathlib,subprocess,hashlib,sys,shutil,re,json
R=pathlib.Path(__file__).resolve().parents[1];E=R/'deps/virtualagc/yaAGC';F=R/'build/cm-pinball-fixture';S=R/'deps/cm-services-fixture'
if not (S/'MAIN.agc.bin').exists():subprocess.run(['python3','tools/cm_services_prepare_reference.py'],cwd=R,check=True)
F.mkdir(exist_ok=True)
for p in S.glob('*.agc'):shutil.copyfile(p,F/p.name)
metadata={}
for name in ['PINBALL_GAME_BUTTONS_AND_LIGHTS.agc','T4RUPT_PROGRAM.agc']:
 a=(R.parent/'Comanche055'/name).read_bytes();original=a.decode()
 b=re.sub(r'^[ \t]*([+-]\d+D?)\t',r' \1\t\t',original,flags=re.M)
 (F/(name+'.original')).write_bytes(a);(F/name).write_text(b)
 metadata[name]={'original_sha256':hashlib.sha256(a).hexdigest(),'indentation_repairs':sum(x!=y for x,y in zip(original.splitlines(),b.splitlines()))}
with (F/'assembly.lst').open('w') as out:subprocess.run([str(R/'deps/virtualagc/yaYUL/yaYUL'),'MAIN.agc'],cwd=F,stdout=out,stderr=subprocess.STDOUT,check=True)
assert (F/'MAIN.agc.bin').read_bytes()==(S/'MAIN.agc.bin').read_bytes()
metadata['rope_sha256']=hashlib.sha256((F/'MAIN.agc.bin').read_bytes()).hexdigest();(R/'build/cm-pinball-source-audit.json').write_text(json.dumps(metadata,indent=2)+'\n')
core=['src/agc_word.c','src/dsky_pinball.c','src/cm_pinball.c','tests/cm_pinball_cases.c']
base=['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests']
subprocess.run(base+core+['tools/cm_pinball_snapshot.c','-o','build/cm-pinball-native'],cwd=R,check=True)
# Only original engine and independent INPUT constructor linked; no native kernels.
subprocess.run(['cc','-std=c99','-O2','-ffunction-sections','-Iinclude','-Itests','-I'+str(E),'tests/cm_pinball_cases.c','src/agc_word.c','tools/cm_pinball_reference.c',str(E/'agc_engine.o'),str(E/'agc_engine_init.o'),str(E/'rfopen.o'),'-Wl,--gc-sections','-o','build/cm-pinball-reference'],cwd=R,check=True)
for kind,cmd in [('native',['./build/cm-pinball-native']),('reference',['./build/cm-pinball-reference',str(F/'MAIN.agc.bin')])]:
 with (R/f'build/cm-pinball-{kind}.snapshots').open('w') as out:subprocess.run(cmd,cwd=R,stdout=out,check=True)
errors=[]
with (R/'build/cm-pinball-native.snapshots').open() as a,(R/'build/cm-pinball-reference.snapshots').open() as b:
 for n,(x,y) in enumerate(zip(a,b)):
  av=list(map(int,x.split()));bv=list(map(int,y.split()));assert len(av)==len(bv)==2052
  if av!=bv and len(errors)<20:errors.append((n,[(oct(i-2),oct(v),oct(w))for i,(v,w)in enumerate(zip(av,bv))if v!=w]))
 assert n==8191
if errors:sys.exit(str(errors))
print('PASS: 8192 complete original CM DSPIN/11DSPIN/2BLANK states, all2048 words plus SUPER/IRQ')
print('Snapshot SHA256:',hashlib.sha256((R/'build/cm-pinball-reference.snapshots').read_bytes()).hexdigest())
