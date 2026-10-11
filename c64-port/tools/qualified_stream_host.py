#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Compare a fresh native producer to every byte of a retained source oracle."""
import argparse,hashlib,lzma,os,pathlib,subprocess
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--binary',required=True);p.add_argument('--archive',required=True);p.add_argument('--sha256',required=True);p.add_argument('--bytes',type=int)
p.add_argument('--arg',action='append',default=[],help='Literal native producer argument; repeat as needed')
a=p.parse_args();root=pathlib.Path(__file__).resolve().parents[1]
child=subprocess.Popen([str(root/a.binary)]+a.arg,cwd=root,stdout=subprocess.PIPE,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
h=hashlib.sha256();n=0
try:
 with lzma.open(root/a.archive,'rb') as reference:
  while expected:=reference.read(65536):
   actual=child.stdout.read(len(expected))
   if actual!=expected:raise RuntimeError('Full source-state stream differs at or after byte '+str(n))
   h.update(actual);n+=len(actual)
  if child.stdout.read(1):raise RuntimeError('Native stream has extra bytes')
 if child.wait()!=0:raise RuntimeError('Native producer failed')
 if h.hexdigest()!=a.sha256:raise RuntimeError('Source corpus decoded SHA256 mismatch')
 if a.bytes is not None and n!=a.bytes:raise RuntimeError('Complete source corpus length mismatch')
finally:
 if child.poll()is None:child.kill();child.wait()
print('PASS all retained source-state bytes:',n,'SHA256',h.hexdigest(),flush=True)
