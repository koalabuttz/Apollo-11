#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Run unchanged Luminary099 P34-35_P74-75 midcourse display subroutines (GET+MGA, GET.LVC, SELECTMU) in the pinned yaAGC oracle
for all 65536 sampled entries from their callers (tests/lm_midgim_cases.c), check the native port against every row, and keep
the oracle stream (build/lm-midgim-reference.snapshots.bin.xz) for the
host and freeze steps. --record rewrites the pinned stream identity."""
import argparse,hashlib,json,lzma,pathlib,struct,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0,str(R/'tools'))
from lm_midgim_core import CORE
import os
STATES=int(os.environ.get('LM_MIDGIM_STATES','65536'));ROW=4106
PIN=R/'evidence/lm-midgim/lm-midgim-source-stream.json'
p=argparse.ArgumentParser(description=__doc__);p.add_argument('--record',action='store_true');a=p.parse_args()
(R/'build').mkdir(exist_ok=True)
if not (R/'build/agc_engine_hardware_dv.o').exists():subprocess.run([sys.executable,'tools/prepare_hardware_dv.py'],cwd=R,check=True)
subprocess.run(['gcc','-std=c99','-O2','-Iinclude','-Itests','-Ideps/virtualagc/yaAGC','-Itools','tools/lm_midgim_reference.c','tests/lm_midgim_cases.c','tests/lm_midgim_debris.c','tests/cm_executive_cases.c','src/agc_word.c','build/agc_engine_hardware_dv.o','deps/virtualagc/yaAGC/agc_engine_init.o','deps/virtualagc/yaAGC/rfopen.o','-lm','-o','build/lm-midgim-reference'],cwd=R,check=True)
subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','tools/lm_midgim_snapshot.c',*CORE,'-lm','-o','build/lm-midgim-native'],cwd=R,check=True)
ref=subprocess.Popen(['build/lm-midgim-reference','deps/lm-rtb-fixture/MAIN.agc.bin',str(STATES)],cwd=R,stdout=subprocess.PIPE)
nat=subprocess.Popen(['build/lm-midgim-native',str(STATES)],cwd=R,stdout=subprocess.PIPE)
h=hashlib.sha256();size=0;status={};out_path=R/'build/lm-midgim-reference.snapshots.bin.xz';tmp=out_path.with_suffix('.xz.part')
with lzma.open(tmp,'wb',preset=3)as out:
 for n in range(STATES):
  x=ref.stdout.read(ROW);y=nat.stdout.read(ROW)
  if len(x)!=ROW or x!=y:
   ref.kill();nat.kill();tmp.unlink()
   if len(x)==ROW==len(y):
    X=struct.unpack('<2053H',x);Y=struct.unpack('<2053H',y)
    print('FAIL state',n,[(f'{i-5:04o}',f'{v:06o}',f'{Y[i]:06o}')for i,v in enumerate(X)if v!=Y[i]][:25])
   else:print('FAIL state',n,'short row')
   raise SystemExit(1)
  status[x[8]]=status.get(x[8],0)+1;out.write(x);h.update(x);size+=len(x)
 if ref.stdout.read(1)or nat.stdout.read(1)or ref.wait()or nat.wait():raise SystemExit('FAIL producer exit or trailing bytes')
tmp.replace(out_path)
identity=dict(archive='build/lm-midgim-reference.snapshots.bin.xz',decoded_sha256=h.hexdigest(),decoded_bytes=size,states=STATES,status_counts={str(k):v for k,v in sorted(status.items())},oracle='pinned yaAGC hardware-DV variant, deps/lm-rtb-fixture/MAIN.agc.bin (unchanged Luminary099)')
if a.record:PIN.write_text(json.dumps(identity,indent=2)+'\n')
pin=json.loads(PIN.read_text())
if (pin['decoded_sha256'],pin['decoded_bytes'])!=(identity['decoded_sha256'],size):raise SystemExit('FAIL oracle stream differs from the pinned identity')
print('PASS65536 original MIDGIM oracle entries equal native 2053-word rows',h.hexdigest(),flush=True)
