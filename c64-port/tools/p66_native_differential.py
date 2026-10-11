#!/usr/bin/env python3
"""Live compiled P66 callbacks against unchanged original assembly boundaries."""
import pathlib,subprocess,sys,hashlib
root=pathlib.Path(__file__).resolve().parents[1];ref=pathlib.Path(sys.argv[1]).resolve();engine=root/'deps/virtualagc/yaAGC'
for name in ('EXECUTIVE.agc','INTERPRETER.agc','LUNAR_LANDING_GUIDANCE_EQUATIONS.agc'):
 assert (root.parent/'Luminary099'/name).read_bytes()==(ref/name).read_bytes()
subprocess.run(['make','-f','Makefile','-f','tools/p66_native.mk','build/p66-native-snapshot'],cwd=root,check=True)
inputs=['tests/'+n+'_cases.c' for n in ('prepare','scale','push','elapsed','store','divide','gravity','velocity','p66_native')]
flags=['-D'+n+'_INPUT_ONLY' for n in ('VELOCITY','GRAVITY','DIVIDE','STORE','ELAPSED','PUSH','SCALE','PREPARE')]
subprocess.run([sys.executable,'tools/prepare_hardware_dv.py'],cwd=root,check=True)
subprocess.run(['gcc','-std=c99','-O2','-DP66_NATIVE_REFERENCE_INPUT']+flags+['-Iinclude','-Itests','-I'+str(engine),'tools/p66_native_reference.c','tests/scenarios.c']+inputs+[str(root/'build/agc_engine_hardware_dv.o'),str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/p66-native-reference'],cwd=root,check=True)
results=[]
for name,cmd in [('native',['./build/p66-native-snapshot']),('reference',['./build/p66-native-reference',str(ref/'MAIN.agc.bin')])]:
 run=subprocess.run(cmd,cwd=root,capture_output=True,text=True,check=True);(root/f'build/p66-native-{name}.snapshots').write_text(run.stdout)
 rows=[list(map(int,r.split())) for r in run.stdout.splitlines()];assert len(rows)==5775 and all(len(r)==119 for r in rows);results.append(rows)
errors=[(a[:2],[(i-2,x,y) for i,(x,y) in enumerate(zip(a,b)) if x!=y]) for a,b in zip(*results) if a!=b]
if errors:sys.exit(str(errors[:20]))
print('PASS: 1155 source inputs, 5775 complete117-word arithmetic snapshots through five compiled native callbacks')
print('Snapshot SHA256:',hashlib.sha256((root/'build/p66-native-reference.snapshots').read_bytes()).hexdigest())

mixed=[]
for name,cmd in [('native',['./build/p66-native-snapshot','--mixed']),('reference',['./build/p66-native-reference',str(ref/'MAIN.agc.bin'),'--mixed'])]:
 run=subprocess.run(cmd,cwd=root,capture_output=True,text=True,check=True);(root/f'build/p66-native-mixed-{name}.snapshots').write_text(run.stdout)
 rows=[list(map(int,r.split())) for r in run.stdout.splitlines()];assert len(rows)==112 and all(len(r)==444 for r in rows);mixed.append(rows)
errors=[(a[:2],[(i-2,x,y) for i,(x,y) in enumerate(zip(a,b)) if x!=y]) for a,b in zip(*mixed) if a!=b]
if errors:sys.exit(str(errors[:20]))
print('PASS: 16 two-P66-job preemptions, 112 complete442-word arithmetic/core/VAC checkpoints')
print('Preempted lower MPAC/MODE/PUSH/OVFIND encoded state survives while higher executes real arithmetic; both SUPERBNK forms')
print('Mixed SHA256:',hashlib.sha256((root/'build/p66-native-mixed-reference.snapshots').read_bytes()).hexdigest())
