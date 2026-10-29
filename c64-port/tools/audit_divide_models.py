#!/usr/bin/env python3
"""Record pinned fast-DV versus its separate hardware-DV model, without native code."""
import hashlib,json,pathlib,subprocess
ROOT=pathlib.Path(__file__).resolve().parents[1]; engine=ROOT/'deps/virtualagc/yaAGC'
subprocess.run(['gcc','-std=c99','-O2','-DDIVIDE_INPUT_ONLY','-DSTORE_INPUT_ONLY','-DELAPSED_INPUT_ONLY','-DPUSH_INPUT_ONLY','-DSCALE_INPUT_ONLY','-DPREPARE_INPUT_ONLY','-I'+str(engine),'-Iinclude','-Itests','tools/divide_reference.c','tests/scenarios.c','tests/prepare_cases.c','tests/scale_cases.c','tests/push_cases.c','tests/elapsed_cases.c','tests/store_cases.c','tests/divide_cases.c',str(engine/'agc_engine.o'),str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/divide-fast-reference'],cwd=ROOT,check=True)
fast=subprocess.run(['./build/divide-fast-reference',str(ROOT/'deps/fixture/MAIN.agc.bin')],cwd=ROOT,check=True,capture_output=True).stdout
hardware=(ROOT/'build/divide-reference.snapshots').read_bytes()
a=[list(map(int,l.split())) for l in fast.splitlines()]; b=[list(map(int,l.split())) for l in hardware.splitlines()]; assert len(a)==len(b)==8098
changes=[]; fields=set()
for x,y in zip(a,b):
 assert x[:4]==y[:4]
 if x!=y:
  positions=[i-4 for i in range(4,97) if x[i]!=y[i]]; fields.update(positions)
  changes.append({'case':x[0],'step':x[1],'stage':x[2],'changed_word_indices':positions,'fast_L_and_MPAC':[x[9],x[29],x[30]],'hardware_L_and_MPAC':[y[9],y[29],y[30]]})
assert fields=={5,25,26} and changes
sample=next(c for c in changes if c['case']==3)
assert sample['hardware_L_and_MPAC']==[15728,32727,17039] and sample['fast_L_and_MPAC']==[11141,32728,21626]
report={'pinned_engine':'1b5d6474841a7ab53237ead070f76f35f162f68d','rows':8098,'fast_snapshot_sha256':hashlib.sha256(fast).hexdigest(),'hardware_snapshot_sha256':hashlib.sha256(hardware).hexdigest(),'differing_rows':len(changes),'changed_modeled_fields':['L','MPAC','MPAC+1'],'cause':'Fast DV SpToDecent receives sign-extended L=-0 as int16_t -1; its equality check against 15-bit -0 (32767) fails, treating zero as a mixed-sign nonzero and decrementing the high dividend. Existing hardware SimulateDV handles the raw register signs directly.','oracle_choice':'Use pinned hardware SimulateDV for every DV. One host-only branch change, no native code or arithmetic linked to either oracle; assembly, rope, pinned checkout and all earlier oracles unchanged.','negative_one_example':{'input_MPAC':[32767,32766],'expected_magnitude_integer_stages':[40,15728],'fast':sample['fast_L_and_MPAC'],'hardware':sample['hardware_L_and_MPAC']},'differences':changes}
(ROOT/'build/divide-dv-model-audit.json').write_text(json.dumps(report,indent=2)+'\n')
print('PASS: two independent pinned DV paths audited over 8098 original-assembly checkpoints')
print(len(changes),'rows differ, only L/MPAC/MPAC+1; sign-extended negative-zero reproducer confirmed')
print('Fast SHA256:',report['fast_snapshot_sha256']); print('Hardware SHA256:',report['hardware_snapshot_sha256'])
