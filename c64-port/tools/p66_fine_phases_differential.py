#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Compare every fine P66 arithmetic/control phase with original assembly."""
from pathlib import Path
import subprocess
import sys
import tempfile
ROOT=Path(__file__).resolve().parents[1]
reference=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else ROOT/'deps/fixture'
subprocess.run([sys.executable,'tools/prepare_hardware_dv.py'],cwd=ROOT,check=True)
for name,targets in [('throttle_feedback_pairs',['test','length-test']),('horizontal_phased',['test'])]:
    source=(ROOT/'tools'/f'{name}.mk').read_text()
    # Preserve the qualified standalone makefile; add only a build-time path parameter.
    source=source.replace('deps/fixture/MAIN.agc.bin','$(REFERENCE)/MAIN.agc.bin')
    with tempfile.NamedTemporaryFile(mode='w',suffix='.mk') as temporary:
        temporary.write(source)
        temporary.flush()
        subprocess.run(['make','-f',temporary.name,*targets,'REFERENCE='+str(reference)],cwd=ROOT,check=True)
