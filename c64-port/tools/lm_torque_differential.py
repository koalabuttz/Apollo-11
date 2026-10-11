#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Independently assemble and compare the complete original torque routine."""
from pathlib import Path
import subprocess
import sys
import tempfile
ROOT=Path(__file__).resolve().parents[1]
# Generate the audited original declaration fixtures required by the standalone
# qualifier. This prerequisite also checks unchanged original source identities.
subprocess.run([sys.executable,'tools/fixed_data_generate.py'],cwd=ROOT,check=True)
source=(ROOT/'tools/lm_torque_host_test.py').read_text()
source=source.replace('ROOT=Path(__file__).resolve().parents[1]',f'ROOT=Path({str(ROOT)!r})')
source=source.replace("f'evidence/lm-torque-{name}.snapshots.xz'","f'build/lm-torque-{name}.snapshots.xz'")
source=source.replace(" run(['python3','tools/freeze_lm_torque.py']);run(['python3','tools/lm_torque_audit.py'])",'')
with tempfile.NamedTemporaryFile(mode='w',suffix='.py') as qualifier:
    qualifier.write(source)
    qualifier.flush()
    subprocess.run([sys.executable,qualifier.name],cwd=ROOT,check=True)
