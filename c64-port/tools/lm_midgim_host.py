#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Compare a fresh strict (or ASAN/UBSAN) native MIDGIM producer with
every byte of the 65536-entry oracle stream and its pinned identity."""
import argparse,json,pathlib,shlex,subprocess,sys
R=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(R/'tools'))
from lm_midgim_core import CORE
p=argparse.ArgumentParser(description=__doc__);p.add_argument('--cc',default='cc');p.add_argument('--sanitize',action='store_true');a=p.parse_args()
flags=shlex.split(a.cc)+['-std=c99','-O1'if a.sanitize else'-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests']
if a.sanitize:flags+=['-g','-fsanitize=address,undefined','-fno-omit-frame-pointer']
binary='build/lm-midgim-'+('sanitize-stream'if a.sanitize else'native')
subprocess.run(flags+['tools/lm_midgim_snapshot.c',*CORE,'-lm','-o',binary],cwd=R,check=True)
pin=json.loads((R/'evidence/lm-midgim/lm-midgim-source-stream.json').read_text())
subprocess.run([sys.executable,'tools/qualified_stream_host.py','--binary',binary,'--archive',pin['archive'],'--sha256',pin['decoded_sha256'],'--bytes',str(pin['decoded_bytes']),'--arg',str(pin['states'])],cwd=R,check=True)
print('PASS65536 original MIDGIM entries'+(' full ASAN/UBSAN'if a.sanitize else' strict native'),flush=True)
