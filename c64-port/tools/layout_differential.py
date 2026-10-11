#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Independently decode retained original rope metadata and check native layout access."""
from pathlib import Path
import hashlib
import shutil
import subprocess
import sys
ROOT=Path(__file__).resolve().parents[1]
for short,mission,sha in [('lm','Luminary099','1f5326e038de5b741b2f27b01ec949dbd688cf1906994e997402587c8628f40e'),('cm','Comanche055','6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061')]:
    rope=ROOT/'evidence'/f'fixed-data-{short}-original.rope.bin'
    assert hashlib.sha256(rope.read_bytes()).hexdigest()==sha
    folder=ROOT/'build/fixed-data'/mission
    folder.mkdir(parents=True,exist_ok=True)
    shutil.copyfile(rope,folder/'MAIN.agc.bin')
subprocess.run([sys.executable,'tools/layout_host_test.py'],cwd=ROOT,check=True)
