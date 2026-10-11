#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Exhaust original fixed-constant accessor addresses against source word map."""
from pathlib import Path
import subprocess,json,struct,hashlib
subprocess.run(['gcc','-O2','-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','tools/imu_mode_transfer_literals.c','src/imu_mode_transfer.c','src/imu_mode_control.c','src/imu_coarse.c','src/agc_word.c','src/agc_multiword.c','-o','build/imu-mode-transfer-literals'],check=True)
s=subprocess.check_output(['build/imu-mode-transfer-literals']);mapping=json.loads(Path('build/imu-mode-transfer-source-map.json').read_text());report={}
for mission,m in enumerate(['lm','cm']):
 lines=[831,842]
 selected={w['physical_fixed_word']:int(w['raw_octal'],8)for w in mapping[m]['words']if w['original_line']in lines};assert len(selected)==2
 for at in range(65536):
  index,found,value=struct.unpack_from('<3H',s,(mission*65536+at)*6)
  assert index==at and found==(at in selected)and value==selected.get(at,0o5555),(m,at,found,value,selected)
 report[m]={'arguments':65536,'original_constants':{f'{at:05o}':f'{v:05o}'for at,v in selected.items()},'result':'PASS'}
report['sha256']=hashlib.sha256(s).hexdigest();Path('build/imu-mode-transfer-literals-proof.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report))
