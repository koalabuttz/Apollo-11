#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma,hashlib,os
R=pathlib.Path(__file__).resolve().parents[1]
core=['src/agc_word.c','src/dsky_nv_consumer.c','src/dsky_nv_admission.c','src/dsky_nv_update.c','src/dsky_nv_blank.c','src/dsky_nv_continuation.c','src/dsky_nv_return.c','src/agc_multiword.c','src/cm_formatter.c','src/dsky_two_digit.c','src/dsky_pinball.c','src/dsky_decimal.c','src/noun_reader.c','src/fixed_data.c','tests/dsky_nv_consumer_cases.c']
subprocess.run(['cc','-O1','-g','-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']+core+['tools/dsky_nv_consumer_snapshot.c','-o','build/dsky-nv-consumer-sanitize'],cwd=R,check=True)
proc=subprocess.Popen(['build/dsky-nv-consumer-sanitize'],cwd=R,stdout=subprocess.PIPE,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'));h=hashlib.sha256();count=0
with lzma.open(R/'build/dsky-nv-consumer-reference.snapshots.xz','rb')as ref:
 for expected in ref:
  actual=proc.stdout.readline();assert actual==expected,count;h.update(actual);count+=1
 assert not proc.stdout.read(1)
assert proc.wait()==0 and count==5888
print('PASS all5888 complete original LM/CM continuous NVSUB paste/noun/blank source states under ASAN/UBSAN, SHA256',h.hexdigest())
