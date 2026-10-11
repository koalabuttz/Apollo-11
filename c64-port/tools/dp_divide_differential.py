#!/usr/bin/env python3
"""Validate generic DDV and BDDV against unchanged historical assembly."""
import hashlib,pathlib,subprocess,sys
from collections import Counter
root=pathlib.Path(__file__).resolve().parents[1]
engine=root/'deps/virtualagc/yaAGC'
reference=pathlib.Path(sys.argv[1]).resolve() if len(sys.argv)>1 else root/'deps/fixture'
assert (root.parent/'Luminary099/INTERPRETER.agc').read_bytes()==(reference/'INTERPRETER.agc').read_bytes()
subprocess.run([sys.executable,'tools/prepare_hardware_dv.py'],cwd=root,check=True)
base=['gcc','-std=c99','-O2','-Iinclude','-Itests']
subprocess.run(base+['tests/dp_divide_snapshot.c','src/guidance_dp_divide.c','src/agc_word.c','src/agc_multiword.c','-o','build/dp-divide-native'],cwd=root,check=True)
subprocess.run(base+['-I'+str(engine),'tools/dp_divide_reference.c','tests/scenarios.c','build/agc_engine_hardware_dv.o',str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/dp-divide-reference'],cwd=root,check=True)
outputs=[]
for name,cmd in [('native',['build/dp-divide-native']),('reference',['build/dp-divide-reference',str(reference/'MAIN.agc.bin')])]:
    run=subprocess.run(cmd,cwd=root,capture_output=True,text=True,check=True)
    (root/f'build/dp-divide-{name}.snapshots').write_text(run.stdout)
    outputs.append([list(map(int,line.split())) for line in run.stdout.splitlines()])
assert len(outputs[0])==len(outputs[1])==163840
errors=[(a,b) for a,b in zip(*outputs) if a!=b]
for a,b in errors[:16]: print('DIFF',a,b)
if errors: sys.exit(f'{len(errors)} differing cases')
print('PASS: 163840 generic DDV/BDDV cases, all 17 modeled words and paths match original assembly with hardware DV')

print("Paths zero/quotient/overflow:",dict(sorted(Counter(r[1] for r in outputs[1]).items())))
print("Near-one flag quotient cases:",sum(r[1]==1 and r[16]==0 for r in outputs[1]))
print("Snapshot SHA256:",hashlib.sha256((root/"build/dp-divide-reference.snapshots").read_bytes()).hexdigest())
