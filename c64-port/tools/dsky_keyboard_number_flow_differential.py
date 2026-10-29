#!/usr/bin/env python3
"""Compare actual actual key IRQ/RESUME/ADVAN/NUDIRECT/CHARIN admission across every physical erasable word."""
import pathlib,subprocess,hashlib,sys,shutil
root=pathlib.Path(__file__).resolve().parents[1];fixture=pathlib.Path(sys.argv[1]).resolve();engine=root/'deps/virtualagc/yaAGC'
# Assemble immutable historical KEYRUPT source under the fixture include name;
# comment/filename differences are allowed only if the entire rope stays equal.
assert (root.parent/'Luminary099/EXECUTIVE.agc').read_bytes()==(fixture/'EXECUTIVE.agc').read_bytes()
private=root/'build/dsky-keyboard-number-flow-fixture';private.mkdir(exist_ok=True)
for f in fixture.glob('*.agc'):shutil.copyfile(f,private/f.name)
shutil.copyfile(root.parent/'Luminary099/KEYRUPT_UPRUPT.agc',private/'KEYRUPT,_UPRUPT.agc')
shutil.copyfile(root.parent/'Luminary099/PINBALL_GAME_BUTTONS_AND_LIGHTS.agc',private/'PINBALL_GAME_BUTTONS_AND_LIGHTS.agc')
run=subprocess.run([str(root/'deps/virtualagc/yaYUL/yaYUL'),'MAIN.agc'],cwd=private,capture_output=True,text=True)
(root/'build/dsky-keyboard-number-flow-assembly.log').write_text(run.stdout+run.stderr)
assert run.returncode==0,run.stdout+run.stderr
assert (private/'MAIN.agc.bin').read_bytes()==(fixture/'MAIN.agc.bin').read_bytes()
fixture=private
core='src/agc_word.c src/agc_multiword.c src/executive.c src/native_dispatch.c src/irq_delivery.c src/waitlist.c src/native_tasks.c src/phase.c src/p66_startup.c src/p66_native.c src/p66_basic.c src/p66_mode_display.c src/dsky_relay.c src/dsky_display.c src/dsky_pinball.c src/dsky_nv_key.c src/dsky_charin_nv.c src/dsky_keycom.c src/dsky_keyrupt.c src/dsky_key_irq.c src/dsky_keyboard.c src/dsky_charin_number.c src/dsky_keyboard_number.c'.split()
subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-DP66_NATIVE_DISPATCH_ONLY','-DP66_BASIC_NO_LAUNCH','-Iinclude','-Itests',*core,'tests/p66_bias_cases.c','tests/dsky_nv_key_cases.c','tests/dsky_keyboard_number_flow_cases.c','tools/dsky_keyboard_number_flow_snapshot.c','-o','build/dsky-keyboard-number-flow-snapshot'],cwd=root,check=True)
subprocess.run(['gcc','-std=c99','-O2','-Iinclude','-Itests','-I'+str(engine),'tools/dsky_keyboard_number_flow_reference.c','tests/scenarios.c','tests/p66_bias_cases.c','tests/dsky_nv_key_cases.c','tests/dsky_keyboard_number_flow_cases.c',str(engine/'agc_engine.o'),str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/dsky-keyboard-number-flow-reference'],cwd=root,check=True)
for name,cmd in [('native',['./build/dsky-keyboard-number-flow-snapshot']),('reference',['./build/dsky-keyboard-number-flow-reference',str(fixture/'MAIN.agc.bin')])]:
 with (root/f'build/dsky-keyboard-number-flow-{name}.snapshots').open('w') as out:subprocess.run(cmd,cwd=root,stdout=out,check=True)
errors=[]
with (root/'build/dsky-keyboard-number-flow-native.snapshots').open() as native,(root/'build/dsky-keyboard-number-flow-reference.snapshots').open() as original:
 for n,(a,b) in enumerate(zip(native,original)):
  av=list(map(int,a.split()));bv=list(map(int,b.split()));assert len(av)==len(bv)==2054
  for f,(x,y) in enumerate(zip(av,bv)):
   if x!=y and len(errors)<60:errors.append(f'case{n} address{f-2:04o}: native{x:06o} original{y:06o}')
 assert n==3071
if errors:sys.exit('\n'.join(errors))
print('PASS: 3072 complete original actual key IRQ/RESUME/ADVAN/NUDIRECT/CHARIN source-entry checkpoints, all2048words plusSUPER/IRQ/channel11')
print('Snapshot SHA256:',hashlib.sha256((root/'build/dsky-keyboard-number-flow-reference.snapshots').read_bytes()).hexdigest())
