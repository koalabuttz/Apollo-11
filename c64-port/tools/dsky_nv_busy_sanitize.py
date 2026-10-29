#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma,hashlib,os
R=pathlib.Path(__file__).resolve().parents[1]
core=['src/agc_word.c','src/dsky_nv_busy.c','tests/dsky_nv_busy_cases.c']
subprocess.run(['cc','-O1','-g','-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']+core+['tools/dsky_nv_busy_snapshot.c','-o','build/dsky-nv-busy-sanitize'],cwd=R,check=True)
proc=subprocess.Popen(['build/dsky-nv-busy-sanitize'],cwd=R,stdout=subprocess.PIPE,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'));h=hashlib.sha256();count=0
with lzma.open(R/'build/dsky-nv-busy-reference.snapshots.xz','rb')as ref:
 for expected in ref:
  actual=proc.stdout.readline();assert actual==expected,count;h.update(actual);count+=1
 assert not proc.stdout.read(1)
assert proc.wait()==0 and count==10240
print('PASS all10240 complete original LM/CM ENDIDLE/NVSUBUSY/PRENVBSY/NVSBWAIT/NVSUBSY1 source states under ASAN/UBSAN, SHA256',h.hexdigest())
