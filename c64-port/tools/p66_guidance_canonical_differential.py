#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Verify canonical-image arithmetic bindings against independent source checkpoints."""
import argparse, pathlib, subprocess, hashlib
root=pathlib.Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--sanitize',action='store_true');a=p.parse_args()
flags=['gcc','-std=c99','-O1' if a.sanitize else '-O2','-Wall','-Wextra','-Werror','-ffunction-sections','-Wl,--gc-sections','-Iinclude','-Itests']
if a.sanitize: flags+=['-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']
binary='p66-guidance-canonical-sanitize' if a.sanitize else 'p66-guidance-canonical-snapshot'
sources=[f'src/guidance_{name}.c' for name in ('elapsed','store','velocity_phased','vector','control_observer','radial','radial_phased','unit','unit_phased','norm','dot','shift','product','sum','acceleration','acceleration_phased','acceleration_store','gravity','dp_divide','projection','projection_phased','trig','rotation')]
subprocess.run(flags+['-DP66_GUIDANCE_ARITH_ONLY','tools/p66_guidance_canonical_snapshot.c','src/p66_guidance_invoke.c','src/p66_native_guidance.c','src/agc_word.c','src/agc_multiword.c']+sources+['-o','build/'+binary],cwd=root,check=True)
defines=['-D'+name+'_INPUT_ONLY' for name in ('PREPARE','SCALE','PUSH','ELAPSED','STORE','DIVIDE','UNIT','ACCELERATION')]
inputs=['tests/'+name+'_cases.c'for name in ('prepare','scale','push','elapsed','store','divide','unit','acceleration')]
subprocess.run(flags+defines+['tools/p66_guidance_acceleration_inputs.c','src/agc_word.c','src/agc_multiword.c']+inputs+['-o','build/p66-guidance-acceleration-inputs'],cwd=root,check=True)
def rows(name):return [list(map(int,l.split()))for l in (root/'build'/name).read_text().splitlines()]
radial=rows('radial-reference.snapshots'); acceleration=rows('acceleration-reference.snapshots'); projection=rows('projection-reference.snapshots'); projection_inputs=rows('projection-reference.inputs')
acc_inputs=[list(map(int,l.split()))for l in subprocess.check_output(['build/p66-guidance-acceleration-inputs'],cwd=root,text=True).splitlines()]
requests=[];expected=[]
def add(group,words,after,status):requests.append([group,len(words)]+words);expected.append((group,after,status))
for before,after in zip(radial[::2],radial[1::2]):
 assert before[:2]==after[:2] and before[2]==0 and after[2]==1
 # Invalid raw MODE/ownership cases belong to the numerical kernel's explicit
 # out-of-scope contract, rather than the runtime admission contract.
 if before[4+32]!=0o77776:continue
 add(6,before[4:],after[4:],0o1302 if after[3]==1 else 0)
for before,after in zip(acc_inputs,acceleration):assert before[0]==after[0];add(7,before[1:],after[2:],0o1302 if after[1]==1 else 0)
for before in projection_inputs:
 n=before[0];words=before[3:]
 for phase in range(3):after=projection[n*3+phase];assert after[:3]==[n,phase,0];add(8+phase,words,after[3:],0);words=after[3:]
payload=''.join(' '.join(map(str,r))+'\n'for r in requests)
run=subprocess.run(['build/'+binary],input=payload,capture_output=True,text=True,cwd=root,check=True)
split=subprocess.run(['build/'+binary,'--split'],input=payload,capture_output=True,text=True,cwd=root,check=True)
assert run.stdout==split.stdout, 'Internal returned phases differ from complete compiled group'
got=[list(map(int,l.split()))for l in run.stdout.splitlines()];assert len(got)==len(expected)
fail=[]
for index,(row,(group,words,status))in enumerate(zip(got,expected)):
 if row != [status]+words:fail.append((index,group,[(i,x,y)for i,(x,y)in enumerate(zip(row,[status]+words))if x!=y]))
if fail:raise SystemExit(str(fail[:12])+f'\n{len(fail)} differing canonical checkpoints')
out=root/'build'/('p66-guidance-canonical-sanitize.snapshots'if a.sanitize else 'p66-guidance-canonical-native.snapshots');out.write_text(run.stdout)
print(f'PASS {len(got)} canonical numerical checkpoints preserve complete source-owned fields and all unowned image words; SHA256 {hashlib.sha256(out.read_bytes()).hexdigest()}')
