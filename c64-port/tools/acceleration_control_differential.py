#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Compare arithmetic control observers to original assembly phase snapshots."""
import pathlib,subprocess,hashlib,sys
root=pathlib.Path(__file__).resolve().parents[1]
flags=['-DACCELERATION_INPUT_ONLY','-DUNIT_INPUT_ONLY','-DDIVIDE_INPUT_ONLY','-DSTORE_INPUT_ONLY','-DELAPSED_INPUT_ONLY','-DPUSH_INPUT_ONLY','-DSCALE_INPUT_ONLY','-DPREPARE_INPUT_ONLY']
cases=['tests/scenarios.c','tests/prepare_cases.c','tests/scale_cases.c','tests/push_cases.c','tests/elapsed_cases.c','tests/store_cases.c','tests/divide_cases.c','tests/unit_cases.c','tests/acceleration_cases.c']
subprocess.run(['gcc','-O2','-std=c99','-Iinclude','-Itests']+flags+cases+['tools/acceleration_phased_inputs.c','-o','build/acceleration-control-inputs'],cwd=root,check=True)
inputs=[list(map(int,x.split()))[1:]for x in subprocess.check_output(['build/acceleration-control-inputs'],cwd=root,text=True).splitlines()]
raw=[list(map(int,x.split()))for x in (root/'build/acceleration-phased-reference.snapshots').read_text().splitlines()]
controls=[int(x.split()[3][1:],8)for x in (root/'build/acceleration-phased-reference.trace').read_text().splitlines()if x.startswith('CTRL')]
assert len(raw)==len(controls)==65536
rows=[]
for i,after in enumerate(raw):
 n,p=after[:2];before=inputs[n]if p==0 else raw[i-1][3:];q=0o1030
 if p:q=controls[i-1]
 w=after[3:];rows.append([p,q,before[25],before[26],before[2],before[3],before[104],before[105],w[95],w[39],controls[i]])
# All16-bit inputs remain literal words; independent native observer reads only
# the physical state needed to account for TC/TPAGREE/GENSCR return debris.
text='\n'.join(' '.join(map(str,x))for x in rows)+'\n';(root/'build/acceleration-control-inputs.snapshots').write_text(text)
subprocess.run(['gcc','-O2','-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','src/agc_word.c','src/agc_multiword.c','src/guidance_control_observer.c','tools/acceleration_control_observer_snapshot.c','-o','build/acceleration-control-observer'],cwd=root,check=True)
out=subprocess.run(['build/acceleration-control-observer'],cwd=root,input=text,capture_output=True,text=True,check=True).stdout
(root/'build/acceleration-control-observer.snapshots').write_text(out)
assert [int(x)for x in out.splitlines()]==controls
seen=set();selected=list(range(64))
for i,r in enumerate(rows):
 key=(r[0],r[-1],r[-3]==0,r[-2]==32767)
 if key not in seen: selected.append(i);seen.add(key)
selected=sorted(set(selected));s='/* SPDX-License-Identifier: MIT */\n#ifndef CONTROL_OBSERVER_VECTORS_H\n#define CONTROL_OBSERVER_VECTORS_H\n#include <stdint.h>\n#define CONTROL_OBSERVER_ROWS '+str(len(selected))+'u\nstatic const uint16_t control_observer_vectors[][11]={\n'
s+='\n'.join(' {'+','.join(map(str,rows[i]))+'},'for i in selected)+'\n};\n#endif\n';
if '--freeze' in sys.argv:(root/'tests/control_observer_vectors.h').write_text(s)
else:assert (root/'tests/control_observer_vectors.h').read_text()==s, 'Frozen source expectations differ; review before --freeze'

print('PASS:65536 original acceleration Q states; frozen',len(selected),'source states; SHA',hashlib.sha256(out.encode()).hexdigest())
