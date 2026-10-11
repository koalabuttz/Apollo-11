#!/usr/bin/env python3
"""Compare ABS/ABVAL/UNIT complete state against unchanged original assembly."""
import hashlib,pathlib,subprocess,sys
from collections import Counter
root=pathlib.Path(__file__).resolve().parents[1];engine=root/'deps/virtualagc/yaAGC'
assert (root.parent/'Luminary099/INTERPRETER.agc').read_bytes()==(root/'deps/fixture/INTERPRETER.agc').read_bytes()
subprocess.run([sys.executable,'tools/prepare_hardware_dv.py'],cwd=root,check=True)
base=['gcc','-std=c99','-O2','-Iinclude','-Itests','-DDIVIDE_INPUT_ONLY','-DSTORE_INPUT_ONLY','-DELAPSED_INPUT_ONLY','-DPUSH_INPUT_ONLY','-DSCALE_INPUT_ONLY','-DPREPARE_INPUT_ONLY']
cases=['tests/scenarios.c','tests/prepare_cases.c','tests/scale_cases.c','tests/push_cases.c','tests/elapsed_cases.c','tests/store_cases.c','tests/divide_cases.c','tests/unit_cases.c']
subprocess.run(base+['tests/unit_snapshot.c','src/guidance_unit.c','src/guidance_norm.c','src/guidance_dp_divide.c','src/agc_word.c','src/agc_multiword.c']+cases+['-o','build/unit-native'],cwd=root,check=True)
subprocess.run(base+['-DUNIT_INPUT_ONLY','-I'+str(engine),'tools/unit_reference.c']+cases+['build/agc_engine_hardware_dv.o',str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/unit-reference'],cwd=root,check=True)
outputs=[]
for name,cmd in [('native',['build/unit-native']),('reference',['build/unit-reference','deps/fixture/MAIN.agc.bin'])]:
 run=subprocess.run(cmd,cwd=root,capture_output=True,text=True,check=True);(root/f'build/unit-{name}.snapshots').write_text(run.stdout);outputs.append([list(map(int,l.split()))for l in run.stdout.splitlines()])
assert len(outputs[0])==len(outputs[1])==50736
errors=[(a,b)for a,b in zip(*outputs)if a!=b]
for a,b in errors[:12]:print('DIFF',a[:2],[(i,x,y)for i,(x,y)in enumerate(zip(a[2:],b[2:]))if x!=y])
if errors:sys.exit(f'{len(errors)} differing cases')
print('PASS:50736 ABS/ABVAL/UNIT cases, all104 complete modeled raw words and paths match unchanged assembly/hardware DV')

print("Operation/path counts:",dict(sorted(Counter((r[0]%3,r[1])for r in outputs[1]).items())))
print("Successful UNIT raw MPTEMP counts:",dict(sorted(Counter(r[41]for r in outputs[1]if r[0]%3==2 and r[1]==0).items())))
print("Snapshot SHA256:",hashlib.sha256((root/"build/unit-reference.snapshots").read_bytes()).hexdigest())
