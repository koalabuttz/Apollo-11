#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma,hashlib,os
R=pathlib.Path(__file__).resolve().parents[1]
core=['src/cm_decimal_pair.c', 'src/cm_pinball.c', 'src/dsky_pinball.c', 'src/agc_word.c', 'src/agc_multiword.c', 'tests/cm_decimal_pair_cases.c']
subprocess.run(['cc','-O1','-g','-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']+core+['tools/cm_decimal_pair_snapshot.c','-o','build/cm-decimal-pair-sanitize'],cwd=R,check=True)
proc=subprocess.Popen(['build/cm-decimal-pair-sanitize'],cwd=R,stdout=subprocess.PIPE,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'));h=hashlib.sha256();count=0
with lzma.open(R/'build/cm-decimal-pair-reference.snapshots.xz','rb')as ref:
 for expected in ref:
  actual=proc.stdout.readline();assert actual==expected,count;h.update(actual);count+=1
 assert not proc.stdout.read(1)
assert proc.wait()==0 and count==1024
print('PASS all1024 complete original cm_decimal_pair source states under ASAN/UBSAN, SHA256',h.hexdigest())
