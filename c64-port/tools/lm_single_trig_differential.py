#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Compare the qualified Luminary original-source corpus in an isolated make scope."""
from pathlib import Path
import subprocess
import sys
ROOT=Path(__file__).resolve().parents[1]
subprocess.run([sys.executable,'tools/prepare_hardware_dv.py'],cwd=ROOT,check=True)
subprocess.run([sys.executable,'tools/lm_single_trig_prepare_reference.py'],cwd=ROOT,check=True)
subprocess.run(['make','-f','tools/lm_single_trig.mk',*['test']],cwd=ROOT,check=True)
