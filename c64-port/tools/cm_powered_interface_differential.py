#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
from pathlib import Path
import subprocess,sys
p=Path(__file__).resolve().parents[1]
for name in ['prepare_hardware_dv','cm_powered_trig_prepare_reference']:subprocess.run([sys.executable,'tools/'+name+'.py'],cwd=p,check=True)
subprocess.run(['make','-f','tools/cm_powered_interface.mk','test','check-host'],cwd=p,check=True)
