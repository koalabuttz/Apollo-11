#!/usr/bin/env python3
"""Compare rotation group complete state against unchanged original assembly."""
import hashlib,pathlib,subprocess,sys
from collections import Counter
root=pathlib.Path(__file__).resolve().parents[1];engine=root/'deps/virtualagc/yaAGC'
assert (root.parent/'Luminary099/INTERPRETER.agc').read_bytes()==(root/'deps/fixture/INTERPRETER.agc').read_bytes()
# Fixture transcription updates comments and SINSLOC spelling, and makes the
# integer DEC scale explicit. The 58 AX*SR*T instruction/data rows are identical.
def kernel_rows(path):
    source=path.read_text();source=source[source.index('\nAX*SR*T\t'):]
    source=source[:source.index('\n\t\tBANK\t10')]
    rows=[' '.join(line.split('#')[0].replace('SINSLOC','SINESLOC').replace(' B-14','').split())for line in source.splitlines()]
    return [row for row in rows if row]
assert kernel_rows(root.parent/'Luminary099/POWERED_FLIGHT_SUBROUTINES.agc')==kernel_rows(root/'deps/fixture/POWERED_FLIGHT_SUBROUTINES.agc')
subprocess.run([sys.executable,'tools/prepare_hardware_dv.py'],cwd=root,check=True)
base=['gcc','-std=c99','-O2','-Iinclude','-Itests','-DDIVIDE_INPUT_ONLY','-DSTORE_INPUT_ONLY','-DELAPSED_INPUT_ONLY','-DPUSH_INPUT_ONLY','-DSCALE_INPUT_ONLY','-DPREPARE_INPUT_ONLY','-DUNIT_INPUT_ONLY']
cases=['tests/scenarios.c','tests/prepare_cases.c','tests/scale_cases.c','tests/push_cases.c','tests/elapsed_cases.c','tests/store_cases.c','tests/divide_cases.c','tests/unit_cases.c','tests/rotation_cases.c']
subprocess.run(base+['tools/rotation_snapshot.c','src/guidance_rotation.c','src/guidance_product.c','src/agc_word.c','src/agc_multiword.c']+cases+['-o','build/rotation-native'],cwd=root,check=True)
subprocess.run(base+['-DROTATION_INPUT_ONLY','-DUNIT_INPUT_ONLY','-I'+str(engine),'tools/rotation_reference.c']+cases+['build/agc_engine_hardware_dv.o',str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/rotation-reference'],cwd=root,check=True)
outputs=[]
for name,cmd in [('native',['build/rotation-native']),('reference',['build/rotation-reference','deps/fixture/MAIN.agc.bin'])]:
 run=subprocess.run(cmd,cwd=root,capture_output=True,text=True,check=True);(root/f'build/rotation-{name}.snapshots').write_text(run.stdout);outputs.append([list(map(int,l.split()))for l in run.stdout.splitlines()])
assert len(outputs[0])==len(outputs[1])==32768
errors=[(a,b)for a,b in zip(*outputs)if a!=b]
for a,b in errors[:12]:print('DIFF',a[:2],[(i,x,y)for i,(x,y)in enumerate(zip(a[2:],b[2:]))if x!=y])
if errors:sys.exit(f'{len(errors)} differing cases')
print('PASS:32768 rotation group cases, all131 complete modeled raw words and paths match unchanged assembly/hardware DV')

print("Paths:",Counter(r[1]for r in outputs[1]))
print("Snapshot SHA256:",hashlib.sha256((root/"build/rotation-reference.snapshots").read_bytes()).hexdigest())
