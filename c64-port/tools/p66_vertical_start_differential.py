#!/usr/bin/env python3
"""Source-entry VRTSTART comparison of the complete physical erasable image."""
import pathlib,subprocess,hashlib,sys,shutil
root=pathlib.Path(__file__).resolve().parents[1];fixture=pathlib.Path(sys.argv[1]).resolve();engine=root/'deps/virtualagc/yaAGC'
for f in ['LUNAR_LANDING_GUIDANCE_EQUATIONS.agc','INTERPRETER.agc']:
 assert (root.parent/'Luminary099'/f).read_bytes()==(fixture/f).read_bytes()
private=root/'build/p66-vertical-start-fixture';private.mkdir(exist_ok=True)
for f in fixture.glob('*.agc'):shutil.copyfile(f,private/f.name)
for f in ['FLAGWORD_ASSIGNMENTS.agc','DAP_INTERFACE_SUBROUTINES.agc','INTER-BANK_COMMUNICATION.agc','SERVICE_ROUTINES.agc']:shutil.copyfile(root.parent/'Luminary099'/f,private/f)
run=subprocess.run([str(root/'deps/virtualagc/yaYUL/yaYUL'),'MAIN.agc'],cwd=private,capture_output=True,text=True)
(root/'build/p66-vertical-start-assembly.log').write_text(run.stdout+run.stderr)
assert run.returncode==0, run.stdout+run.stderr
assert (private/'MAIN.agc.bin').read_bytes()==(fixture/'MAIN.agc.bin').read_bytes()
fixture=private
subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','src/agc_word.c','src/executive.c','src/waitlist.c','src/phase.c','src/p66_startup.c','src/p66_initialization.c','src/p66_vertical_start.c','tests/p66_bias_cases.c','tests/p66_vertical_start_cases.c','tools/p66_vertical_start_snapshot.c','-o','build/p66-vertical-start-snapshot'],cwd=root,check=True)
subprocess.run(['gcc','-std=c99','-O2','-Iinclude','-Itests','-I'+str(engine),'tools/p66_vertical_start_reference.c','tests/scenarios.c','tests/p66_bias_cases.c','tests/p66_vertical_start_cases.c',str(engine/'agc_engine.o'),str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/p66-vertical-start-reference'],cwd=root,check=True)
for name,cmd in [('native',['./build/p66-vertical-start-snapshot']),('reference',['./build/p66-vertical-start-reference',str(fixture/'MAIN.agc.bin')])]:
 with (root/f'build/p66-vertical-start-{name}.snapshots').open('w') as out,(root/f'build/p66-vertical-start-{name}.trace').open('w') as err:subprocess.run(cmd,cwd=root,stdout=out,stderr=err,check=True)
errors=[]
with (root/'build/p66-vertical-start-native.snapshots').open() as native,(root/'build/p66-vertical-start-reference.snapshots').open() as original:
 for n,(a,b) in enumerate(zip(native,original)):
  av=list(map(int,a.split()));bv=list(map(int,b.split()));assert len(av)==2052 and len(bv)==2052
  for f,(x,y) in enumerate(zip(av,bv)):
   if x!=y and len(errors)<60:errors.append(f'case{n} address{f-2:04o}: native{x:06o} original{y:06o}')
 assert n==1023
if errors:sys.exit('\n'.join(errors))
print('PASS: 1,024 complete VRTSTART source-entry checkpoints, all2,048erasablewords plusSUPERBNK/IRQ')
print('STOPRATE, flag clears, POSTJUMP, PHASCHNG, TWIDDLE; stops RODCOMP or BAILOUT1')
print('Snapshot SHA256:',hashlib.sha256((root/'build/p66-vertical-start-reference.snapshots').read_bytes()).hexdigest())
