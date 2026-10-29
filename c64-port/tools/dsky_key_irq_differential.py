#!/usr/bin/env python3
"""Compare actual key IRQ admission across every physical erasable word."""
import pathlib,subprocess,hashlib,sys,shutil
root=pathlib.Path(__file__).resolve().parents[1];fixture=pathlib.Path(sys.argv[1]).resolve();engine=root/'deps/virtualagc/yaAGC'
# Assemble immutable historical KEYRUPT source under the fixture include name;
# comment/filename differences are allowed only if the entire rope stays equal.
assert (root.parent/'Luminary099/EXECUTIVE.agc').read_bytes()==(fixture/'EXECUTIVE.agc').read_bytes()
private=root/'build/dsky-key-irq-fixture';private.mkdir(exist_ok=True)
for f in fixture.glob('*.agc'):shutil.copyfile(f,private/f.name)
shutil.copyfile(root.parent/'Luminary099/KEYRUPT_UPRUPT.agc',private/'KEYRUPT,_UPRUPT.agc')
shutil.copyfile(root.parent/'Luminary099/PINBALL_GAME_BUTTONS_AND_LIGHTS.agc',private/'PINBALL_GAME_BUTTONS_AND_LIGHTS.agc')
for name in ['INTERRUPT_LEAD_INS.agc','WAITLIST.agc']:
 assert (root.parent/'Luminary099'/name).read_bytes()==(fixture/name).read_bytes()
run=subprocess.run([str(root/'deps/virtualagc/yaYUL/yaYUL'),'MAIN.agc'],cwd=private,capture_output=True,text=True)
(root/'build/dsky-key-irq-assembly.log').write_text(run.stdout+run.stderr)
assert run.returncode==0,run.stdout+run.stderr
assert (private/'MAIN.agc.bin').read_bytes()==(fixture/'MAIN.agc.bin').read_bytes()
fixture=private
core='src/agc_word.c src/executive.c src/dsky_keycom.c src/dsky_keyrupt.c src/dsky_key_irq.c'.split()
subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests',*core,'tests/dsky_key_irq_cases.c','tools/dsky_key_irq_snapshot.c','-o','build/dsky-key-irq-snapshot'],cwd=root,check=True)
subprocess.run(['gcc','-std=c99','-O2','-Iinclude','-Itests','-I'+str(engine),'tools/dsky_key_irq_reference.c','tests/scenarios.c','tests/dsky_key_irq_cases.c',str(engine/'agc_engine.o'),str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/dsky-key-irq-reference'],cwd=root,check=True)
for name,cmd in [('native',['./build/dsky-key-irq-snapshot']),('reference',['./build/dsky-key-irq-reference',str(fixture/'MAIN.agc.bin')])]:
 with (root/f'build/dsky-key-irq-{name}.snapshots').open('w') as out:subprocess.run(cmd,cwd=root,stdout=out,check=True)
errors=[]
with (root/'build/dsky-key-irq-native.snapshots').open() as native,(root/'build/dsky-key-irq-reference.snapshots').open() as original:
 for n,(a,b) in enumerate(zip(native,original)):
  av=list(map(int,a.split()));bv=list(map(int,b.split()));assert len(av)==len(bv)==2053
  for f,(x,y) in enumerate(zip(av,bv)):
   if x!=y and len(errors)<60:errors.append(f'case{n} address{f-2:04o}: native{x:06o} original{y:06o}')
 assert n==4095
if errors:sys.exit('\n'.join(errors))
print('PASS: 4096 complete original key IRQ source-entry checkpoints, all 2048 words plus SUPER/IRQ/ISR')
print('Snapshot SHA256:',hashlib.sha256((root/'build/dsky-key-irq-reference.snapshots').read_bytes()).hexdigest())
