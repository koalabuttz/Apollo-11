#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
from pathlib import Path
import subprocess,sys
p=Path(__file__).resolve().parents[1]
for name in ['prepare_hardware_dv','cm_powered_trig_prepare_reference']:subprocess.run([sys.executable,'tools/'+name+'.py'],cwd=p,check=True)
for name in ['cm_smcdure','cm_nbsm','cm_powered_quick','cm_powered_fixed_return']:subprocess.run(['make','-f','tools/'+name+'.mk','test','check-host'],cwd=p,check=True)
subprocess.run([sys.executable,'tools/cm_powered_file_review.py'],cwd=p,check=True)
