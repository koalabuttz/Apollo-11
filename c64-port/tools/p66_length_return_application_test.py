#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import argparse,hashlib,itertools,lzma,os,subprocess
from pathlib import Path
from p66_application_port_host import inventories
root=Path(__file__).resolve().parents[1];p=argparse.ArgumentParser();p.add_argument('--sanitize',action='store_true');a=p.parse_args();_,sources,inputs,defines=inventories()[2];binary=root/('build/p66-length-return-application'+('-sanitize'if a.sanitize else''));flags=['gcc','-std=c99','-O1'if a.sanitize else'-O2','-Wall','-Wextra','-Werror','-Iinclude','-Itests','-ffunction-sections','-fdata-sections','-Wl,--gc-sections']+defines
if a.sanitize:flags+=['-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']
subprocess.run(flags+list(dict.fromkeys(sources+inputs))+['src/p66_length_return_control.c','tools/p66_length_return_application_snapshot.c','-o',str(binary)],cwd=root,check=True);h=hashlib.sha256()
with lzma.open(root/'evidence/p66-application-live-source.snapshots.xz','rb')as expected:
 process=subprocess.Popen([str(binary)],cwd=root,stdout=subprocess.PIPE,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
 try:
  for n,(x,y)in enumerate(itertools.zip_longest(expected,process.stdout),1):assert x==y,(n,x[:80]if x else None,y[:80]if y else None);h.update(x)
  assert process.wait()==0
 finally:
  if process.poll()is None:process.kill();process.wait()
  process.stdout.close()
assert n==1226 and h.hexdigest()=='9ea8e8bc75b90a91ac5185b2546cf93ca47dc786b691633527df6959819e22dc'
print('PASS: additive length/store source-return controls preserve all1226 immutable raw application states;',h.hexdigest())
