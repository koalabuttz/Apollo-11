#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma,hashlib,os
R=pathlib.Path(__file__).resolve().parents[1]
core=['src/agc_word.c','src/agc_multiword.c','src/dsky_pinball.c','src/dsky_decimal_raw.c','tests/dsky_decimal_raw_cases.c','tests/dsky_decimal_general_cases.c']
subprocess.run(['cc','-O1','-g','-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']+core+['tools/dsky_decimal_raw_snapshot.c','-o','build/dsky-decimal-raw-sanitize'],cwd=R,check=True)
proc=subprocess.Popen(['build/dsky-decimal-raw-sanitize'],cwd=R,stdout=subprocess.PIPE,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'));h=hashlib.sha256();count=0
with lzma.open(R/'build/dsky-decimal-raw-reference.snapshots.xz','rb')as ref:
 for expected in ref:
  actual=proc.stdout.readline();assert actual==expected,count;h.update(actual);count+=1
 assert not proc.stdout.read(1)
assert proc.wait()==0 and count==262144
print('PASS all262144 complete original LM/CM decimal formatter caller/CCSHOLE source states under ASAN/UBSAN, SHA256',h.hexdigest())
