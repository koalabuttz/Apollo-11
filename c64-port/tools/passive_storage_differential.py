#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
from pathlib import Path
import subprocess,sys
p=Path(__file__).resolve().parents[1]
s=(p/'tools/passive_storage_audit.py').read_text().replace("out=R/'evidence/passive-storage-host-audit.json'", "out=R/'build/passive-storage-reproduced-host-audit.json'")
q=p/'tools/.passive_storage_reproduce.py'
try:
 q.write_text(s);subprocess.run([sys.executable,str(q)],cwd=p,check=True)
finally:q.unlink(missing_ok=True)
