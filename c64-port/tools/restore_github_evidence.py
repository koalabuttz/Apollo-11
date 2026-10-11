#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Restore exact frozen archives from ordinary GitHub-sized transport parts."""
from pathlib import Path
import argparse,hashlib,json,os,tempfile
ROOT=Path(__file__).resolve().parents[1]
def digest(path):
 h=hashlib.sha256();n=0
 with path.open('rb')as f:
  while data:=f.read(1048576):h.update(data);n+=len(data)
 return n,h.hexdigest()
def restore(root=ROOT):
 manifest=root/'evidence/github-evidence-transport.json'
 if not manifest.exists():return 0
 data=json.loads(manifest.read_text());restored=0
 for name,key in data['files'].items():
  dst=root/name;obj=data['objects'][key]
  if dst.exists():continue
  fd,tmp=tempfile.mkstemp(prefix='github-evidence-',dir=dst.parent);h=hashlib.sha256();size=0
  try:
   with os.fdopen(fd,'wb')as out:
    for part in obj['parts']:
     src=root/part['path'];assert digest(src)==(part['bytes'],part['sha256']),part['path']
     with src.open('rb')as inp:
      while chunk:=inp.read(1048576):out.write(chunk);h.update(chunk);size+=len(chunk)
   assert(size,h.hexdigest())==(obj['bytes'],obj['sha256']),name
   os.replace(tmp,dst);restored+=1
  finally:
   if os.path.exists(tmp):os.unlink(tmp)
 return restored
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--root',type=Path,default=ROOT);p.add_argument('--verify',action='store_true');a=p.parse_args();n=restore(a.root)
 if a.verify:
  data=json.loads((a.root/'evidence/github-evidence-transport.json').read_text())
  for name,key in data['files'].items():obj=data['objects'][key];assert digest(a.root/name)==(obj['bytes'],obj['sha256']),name
 print('PASS byte-exact GitHub evidence transport',n,'archives restored')
