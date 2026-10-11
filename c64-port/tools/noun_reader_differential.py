#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Run the complete original noun-reader qualification without changing retained evidence."""
from pathlib import Path
import hashlib
import shutil
import subprocess
import sys
import tempfile
ROOT=Path(__file__).resolve().parents[1]
for short,mission,sha in [('lm','Luminary099','1f5326e038de5b741b2f27b01ec949dbd688cf1906994e997402587c8628f40e'),('cm','Comanche055','6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061')]:
    rope=ROOT/'evidence'/f'fixed-data-{short}-original.rope.bin'
    assert hashlib.sha256(rope.read_bytes()).hexdigest()==sha
    folder=ROOT/'build/fixed-data'/mission
    folder.mkdir(parents=True,exist_ok=True)
    shutil.copyfile(rope,folder/'MAIN.agc.bin')
# Keep the reviewed qualifier byte-identical; redirect only its regenerated
# evidence output and omit regeneration of already frozen test fixtures.
source=(ROOT/'tools/noun_reader_host_test.py').read_text()
source=source.replace('ROOT = Path(__file__).resolve().parents[1]',f'ROOT = Path({str(ROOT)!r})')
source=source.replace("f'evidence/noun-reader-{name}.snapshots.xz'","f'build/noun-reader-{name}.snapshots.xz'")
source=source.replace("    run(['python3', 'tools/freeze_noun_reader.py'])",'')
with tempfile.NamedTemporaryFile(mode='w',suffix='.py') as qualifier:
    qualifier.write(source)
    qualifier.flush()
    subprocess.run([sys.executable,qualifier.name],cwd=ROOT,check=True)
