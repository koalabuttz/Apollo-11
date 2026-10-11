#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Recheck immutable prefix and final extension, keeping a new evidence log."""
import pathlib,subprocess,os,sys
root=pathlib.Path(__file__).resolve().parents[1]
commands=[['make','test-p66-native'],['make','sanitize-p66-native'],['python3','tools/p66_native_differential.py','deps/fixture'],['python3','tools/p66_guidance_canonical_differential.py'],['python3','tools/p66_guidance_canonical_differential.py','--sanitize'],['python3','tools/p66_guidance_live_differential.py','--sanitize'],['python3','tools/p66_guidance_pending_differential.py','--sanitize']]
with(root/'evidence/p66-guidance-regression.txt').open('w')as log:
 for command in commands:
  line='$ '+' '.join(command)+'\n';print(line,end='',flush=True);log.write(line);log.flush()
  result=subprocess.run(command,cwd=root,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'),capture_output=True,text=True)
  print(result.stdout,end='',flush=True);print(result.stderr,end='',file=sys.stderr,flush=True);log.write(result.stdout+result.stderr);log.flush()
  if result.returncode:raise SystemExit(result.returncode)
print('PASS: immutable prefix and all final extended source checkpoints pass strict and sanitizer regression')
