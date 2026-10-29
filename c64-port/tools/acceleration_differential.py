#!/usr/bin/env python3
"""Compare acceleration group complete state against unchanged original assembly."""
import hashlib,pathlib,subprocess,sys
from collections import Counter
root=pathlib.Path(__file__).resolve().parents[1];engine=root/'deps/virtualagc/yaAGC'
assert (root.parent/'Luminary099/INTERPRETER.agc').read_bytes()==(root/'deps/fixture/INTERPRETER.agc').read_bytes()
subprocess.run([sys.executable,'tools/prepare_hardware_dv.py'],cwd=root,check=True)
base=['gcc','-std=c99','-O2','-Iinclude','-Itests','-DDIVIDE_INPUT_ONLY','-DSTORE_INPUT_ONLY','-DELAPSED_INPUT_ONLY','-DPUSH_INPUT_ONLY','-DSCALE_INPUT_ONLY','-DPREPARE_INPUT_ONLY']
cases=['tests/scenarios.c','tests/prepare_cases.c','tests/scale_cases.c','tests/push_cases.c','tests/elapsed_cases.c','tests/store_cases.c','tests/divide_cases.c','tests/unit_cases.c','tests/acceleration_cases.c']
subprocess.run(base+['tools/acceleration_snapshot.c','src/guidance_acceleration.c','src/guidance_unit.c','src/guidance_sum.c','src/guidance_shift.c','src/guidance_gravity.c','src/guidance_norm.c','src/guidance_dp_divide.c','src/agc_word.c','src/agc_multiword.c']+cases+['-o','build/acceleration-native'],cwd=root,check=True)
subprocess.run(base+['-DACCELERATION_INPUT_ONLY','-DUNIT_INPUT_ONLY','-I'+str(engine),'tools/acceleration_reference.c']+cases+['build/agc_engine_hardware_dv.o',str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/acceleration-reference'],cwd=root,check=True)
outputs=[]
for name,cmd in [('native',['build/acceleration-native']),('reference',['build/acceleration-reference','deps/fixture/MAIN.agc.bin'])]:
 run=subprocess.run(cmd,cwd=root,capture_output=True,text=True,check=True);(root/f'build/acceleration-{name}.snapshots').write_text(run.stdout);outputs.append([list(map(int,l.split()))for l in run.stdout.splitlines()])
assert len(outputs[0])==len(outputs[1])==16384
errors=[(a,b)for a,b in zip(*outputs)if a!=b]
for a,b in errors[:12]:print('DIFF',a[:2],[(i,x,y)for i,(x,y)in enumerate(zip(a[2:],b[2:]))if x!=y])
if errors:sys.exit(f'{len(errors)} differing cases')
print('PASS:16384 acceleration group cases, all112 complete modeled raw words and paths match unchanged assembly/hardware DV')

print("Paths:",Counter(r[1]for r in outputs[1]))
print("Snapshot SHA256:",hashlib.sha256((root/"build/acceleration-reference.snapshots").read_bytes()).hexdigest())
