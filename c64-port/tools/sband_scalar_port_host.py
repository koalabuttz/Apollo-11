#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Exact fresh native comparison against ordered canonical original chunks."""
import argparse,hashlib,json,lzma,os,pathlib,subprocess
R=pathlib.Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--binary',required=True);a=p.parse_args();x=json.loads((R/'evidence/sband-scalar-proof.json').read_text())['corpus']
child=subprocess.Popen([str(R/a.binary)],cwd=R,stdout=subprocess.PIPE,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'));h=hashlib.sha256();total=0
try:
 for chunk in x['canonical_original_archives']:
  path=R/chunk['path'];assert hashlib.sha256(path.read_bytes()).hexdigest()==chunk['compressed_sha256'];ch=hashlib.sha256();size=0
  with lzma.open(path,'rb')as source:
   while expected:=source.read(65536):
    actual=child.stdout.read(len(expected))
    if actual!=expected:raise RuntimeError('Native full state differs at byte '+str(total))
    h.update(actual);ch.update(actual);total+=len(actual);size+=len(actual)
  assert size==chunk['decoded_bytes']and ch.hexdigest()==chunk['decoded_sha256']
 assert not child.stdout.read(1)and child.wait()==0 and total==x['decoded_bytes']and h.hexdigest()==x['decoded_sha256']
finally:
 if child.poll()is None:child.kill();child.wait()
print('PASS all 786,432 original canonical scalar DMP/DAD/DSU states;',total,'bytes;',h.hexdigest())
