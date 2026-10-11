#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma,hashlib,os
R=pathlib.Path(__file__).resolve().parents[1]
core=['src/cm_minutes_seconds.c', 'src/cm_decimal_pair.c', 'src/cm_pinball.c', 'src/dsky_pinball.c', 'src/cm_interbank.c', 'src/fixed_data.c', 'src/guidance_product.c', 'src/agc_word.c', 'src/agc_multiword.c', 'tests/cm_minutes_seconds_cases.c']
subprocess.run(['cc','-O1','-g','-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']+core+['tools/cm_minutes_seconds_snapshot.c','-o','build/cm-minutes-seconds-sanitize'],cwd=R,check=True)
proc=subprocess.Popen(['build/cm-minutes-seconds-sanitize'],cwd=R,stdout=subprocess.PIPE,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'));h=hashlib.sha256();count=0
with lzma.open(R/'build/cm-minutes-seconds-reference.snapshots.xz','rb')as ref:
 for expected in ref:
  actual=proc.stdout.readline();assert actual==expected,count;h.update(actual);count+=1
 assert not proc.stdout.read(1)
assert proc.wait()==0 and count==3072
print('PASS all3072 complete original cm_minutes_seconds source states under ASAN/UBSAN, SHA256',h.hexdigest())
