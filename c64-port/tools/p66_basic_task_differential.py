#!/usr/bin/env python3
"""Compare actual RODTASK admission across every physical erasable word."""
import pathlib,subprocess,hashlib,sys
root=pathlib.Path(__file__).resolve().parents[1];fixture=pathlib.Path(sys.argv[1]).resolve();engine=root/'deps/virtualagc/yaAGC'
# All three source bodies are historical; retained rope assembled unchanged.
for name in ['LUNAR_LANDING_GUIDANCE_EQUATIONS.agc','EXECUTIVE.agc','WAITLIST.agc']:
 assert (root.parent/'Luminary099'/name).read_bytes()==(fixture/name).read_bytes()
core='src/agc_word.c src/agc_multiword.c src/executive.c src/native_dispatch.c src/irq_delivery.c src/waitlist.c src/native_tasks.c src/phase.c src/p66_startup.c src/p66_native.c src/p66_mode_display.c src/dsky_relay.c src/dsky_display.c src/p66_basic.c'.split()
subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-DP66_NATIVE_DISPATCH_ONLY','-DP66_BASIC_NO_LAUNCH','-Iinclude','-Itests',*core,'tests/p66_basic_task_cases.c','tools/p66_basic_task_snapshot.c','-o','build/p66-basic-task-snapshot'],cwd=root,check=True)
subprocess.run(['gcc','-std=c99','-O2','-Iinclude','-Itests','-I'+str(engine),'tools/p66_basic_task_reference.c','tests/scenarios.c','tests/p66_basic_task_cases.c',str(engine/'agc_engine.o'),str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/p66-basic-task-reference'],cwd=root,check=True)
for name,cmd in [('native',['./build/p66-basic-task-snapshot']),('reference',['./build/p66-basic-task-reference',str(fixture/'MAIN.agc.bin')])]:
 with (root/f'build/p66-basic-task-{name}.snapshots').open('w') as out:subprocess.run(cmd,cwd=root,stdout=out,check=True)
errors=[]
with (root/'build/p66-basic-task-native.snapshots').open() as native,(root/'build/p66-basic-task-reference.snapshots').open() as original:
 for n,(a,b) in enumerate(zip(native,original)):
  av=list(map(int,a.split()));bv=list(map(int,b.split()));assert len(av)==len(bv)==2052
  for f,(x,y) in enumerate(zip(av,bv)):
   if x!=y and len(errors)<60:errors.append(f'case{n} address{f-2:04o}: native{x:06o} original{y:06o}')
 assert n==2047
if errors:sys.exit('\n'.join(errors))
print('PASS: 2048 complete original RODTASK source-entry checkpoints, all2048words plusSUPER/IRQ')
print('Snapshot SHA256:',hashlib.sha256((root/'build/p66-basic-task-reference.snapshots').read_bytes()).hexdigest())
