#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import subprocess,struct,pathlib,re,json,hashlib
R=pathlib.Path(__file__).resolve().parents[1]
subprocess.run(['cc','-O2','-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','tools/dsky_monitor_literals.c','src/dsky_monitor.c','src/agc_word.c','-o','build/dsky-monitor-literals'],cwd=R,check=True)
s=subprocess.check_output(['build/dsky-monitor-literals'],cwd=R);report={}
pcs=[0o3237,0o3267,0o3270,0o3305,0o3306,0o3311,0o3312,0o3320,0o3347,0o3350]
for mission,f in enumerate(['lm','cm']):
 selected={}
 for line in (R/'deps'/f'{f}-powered-fixture'/'assembly.lst').read_text().splitlines():
  m=re.match(r'^\d+,\d+: 41,([0-7]{4})\s+([0-7]{5})(?: ([0-7]{5}))?\s+',line)
  if m:
   pc=int(m[1],8)
   for offset,raw in enumerate(m.groups()[1:]):
    if raw is not None and pc+offset in pcs:selected[0o41*0o2000+pc+offset-0o2000]=int(raw,8)
 assert len(selected)==10
 for at in range(65536):
  index,found,value=struct.unpack_from('<3H',s,(mission*65536+at)*6)
  assert index==at and found==(at in selected)and value==selected.get(at,0o5555),(f,at,found,value)
 report[f]={'arguments':65536,'constants':{f'{at:06o}':f'{v:05o}'for at,v in selected.items()},'result':'PASS'}
report['sha256']=hashlib.sha256(s).hexdigest();(R/'build/dsky-monitor-literals-proof.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report))
