#!/usr/bin/env python3
"""Original Pinball source-entry comparison of the complete physical erasable image."""
import pathlib,subprocess,hashlib,sys,shutil
root=pathlib.Path(__file__).resolve().parents[1];fixture=pathlib.Path(sys.argv[1]).resolve();engine=root/'deps/virtualagc/yaAGC'
for f in ['INTERPRETER.agc']:
 assert (root.parent/'Luminary099'/f).read_bytes()==(fixture/f).read_bytes()
private=root/'build/dsky-nv-key-fixture';private.mkdir(exist_ok=True)
for f in fixture.glob('*.agc'):shutil.copyfile(f,private/f.name)
for f in ['PINBALL_GAME_BUTTONS_AND_LIGHTS.agc','T4RUPT_PROGRAM.agc']:shutil.copyfile(root.parent/'Luminary099'/f,private/f)
run=subprocess.run([str(root/'deps/virtualagc/yaYUL/yaYUL'),'MAIN.agc'],cwd=private,capture_output=True,text=True)
(root/'build/dsky-nv-key-assembly.log').write_text(run.stdout+run.stderr)
assert run.returncode==0, run.stdout+run.stderr
assert (private/'MAIN.agc.bin').read_bytes()==(fixture/'MAIN.agc.bin').read_bytes()
fixture=private
subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','src/agc_word.c','src/dsky_pinball.c','src/dsky_nv_key.c','tests/p66_bias_cases.c','tests/dsky_nv_key_cases.c','tools/dsky_nv_key_snapshot.c','-o','build/dsky-nv-key-snapshot'],cwd=root,check=True)
subprocess.run(['gcc','-std=c99','-O2','-Iinclude','-Itests','-I'+str(engine),'tools/dsky_nv_key_reference.c','tests/scenarios.c','tests/p66_bias_cases.c','tests/dsky_nv_key_cases.c',str(engine/'agc_engine.o'),str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/dsky-nv-key-reference'],cwd=root,check=True)
for name,cmd in [('native',['./build/dsky-nv-key-snapshot']),('reference',['./build/dsky-nv-key-reference',str(fixture/'MAIN.agc.bin')])]:
 with (root/f'build/dsky-nv-key-{name}.snapshots').open('w') as out,(root/f'build/dsky-nv-key-{name}.trace').open('w') as err:subprocess.run(cmd,cwd=root,stdout=out,stderr=err,check=True)
errors=[]
with (root/'build/dsky-nv-key-native.snapshots').open() as native,(root/'build/dsky-nv-key-reference.snapshots').open() as original:
 for n,(a,b) in enumerate(zip(native,original)):
  av=list(map(int,a.split()));bv=list(map(int,b.split()));assert len(av)==2052 and len(bv)==2052
  for f,(x,y) in enumerate(zip(av,bv)):
   if x!=y and len(errors)<60:errors.append(f'case{n} address{f-2:04o}: native{x:06o} original{y:06o}')
 assert n==4095
if errors:sys.exit('\n'.join(errors))
print('PASS: 4,096 complete VERB/NOUN source-entry checkpoints, all2,048erasablewords plusSUPERBNK/IRQ')
print('All8 retained erasablebanks: original VERB/NOUN physical relay and input stores; stops actual ENDOFJOB entry')
print('Snapshot SHA256:',hashlib.sha256((root/'build/dsky-nv-key-reference.snapshots').read_bytes()).hexdigest())
