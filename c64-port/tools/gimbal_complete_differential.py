#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
from pathlib import Path
import subprocess,sys
p=Path(__file__).resolve().parents[1]
s=(p/'tools/gimbal_complete_host_test.py').read_text().replace("ROOT/f'evidence/gimbal-complete-", "ROOT/f'build/gimbal-complete-reproduced-").replace("run(['python3','tools/freeze_gimbal_complete.py'])", "pass")
q=p/'tools/.gimbal_complete_reproduce.py'
try:
 q.write_text(s);subprocess.run([sys.executable,str(q)],cwd=p,check=True)
finally:q.unlink(missing_ok=True)
