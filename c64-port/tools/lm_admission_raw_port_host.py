#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Replay retained whole canonical admission states in a private host build."""
import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
from native_resident_port_host import read_manifest
ROOT=Path(__file__).resolve().parents[1]
CORE=['src/agc_word.c','src/executive.c','src/p66_native.c','src/lm_admission_raw.c',
      'tests/lm_admission_image_cases.c','tests/lm_admission_raw_cases.c']
def main():
 p=argparse.ArgumentParser();p.add_argument('--sanitize',action='store_true');args=p.parse_args()
 assert (ROOT.parent/'Luminary099/EXECUTIVE.agc').read_bytes()==(ROOT/'deps/fixture/EXECUTIVE.agc').read_bytes()
 read_manifest('lm-admission-raw-modules.json',32)
 with tempfile.TemporaryDirectory(prefix='lm-admission-raw-port-',dir=ROOT/'build')as d:
  w=Path(d)
  for name in ('src','include','tests','evidence'):(w/name).symlink_to(ROOT/name,target_is_directory=True)
  (w/'build').mkdir();(w/'tools').mkdir()
  for name in ('lm_admission_raw_differential.py','lm_admission_raw_snapshot.c'):
   shutil.copyfile(ROOT/'tools'/name,w/'tools'/name)
  command=[sys.executable,str(w/'tools/lm_admission_raw_differential.py'),'--retained']
  flags=['cc','-DP66_NATIVE_DISPATCH_ONLY','-ffunction-sections','-fdata-sections','-Wl,--gc-sections',
         '-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests',
         '-O1'if args.sanitize else'-O2']
  if args.sanitize:command+=['--sanitize'];flags+=['-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']
  subprocess.run(command,cwd=w,check=True)
  binary=w/'build/guards'
  subprocess.run(flags+CORE+['src/lm_admission_image.c','tests/test_lm_admission_raw.c','-o',str(binary)],cwd=w,check=True)
  subprocess.run([str(binary)],cwd=w,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'),check=True)
 print('PASS:retained raw admission '+('sanitize'if args.sanitize else'strict')+';16384 whole original states,16384 old wrapper comparisons,32 checked PRGs')
if __name__=='__main__':main()
