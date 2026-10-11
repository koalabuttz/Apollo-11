#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma,hashlib,os
R=pathlib.Path(__file__).resolve().parents[1]
core=['src/agc_word.c','src/dsky_fixed_display.c','src/dsky_fixed_display_data.c','src/lm_fixed_compat.c','src/cm_fixed_compat.c','src/lm_interbank.c','src/cm_fixed_callers.c','src/dsky_octal_display.c','src/dsky_pinball.c','tests/dsky_fixed_display_cases.c']
subprocess.run(['cc','-O1','-g','-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']+core+['tools/dsky_fixed_display_snapshot.c','-o','build/dsky-fixed-display-sanitize'],cwd=R,check=True)
proc=subprocess.Popen(['build/dsky-fixed-display-sanitize'],cwd=R,stdout=subprocess.PIPE,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'));h=hashlib.sha256();count=0
with lzma.open(R/'build/dsky-fixed-display-reference.snapshots.xz','rb')as ref:
 for expected in ref:
  actual=proc.stdout.readline();assert actual==expected,count;h.update(actual);count+=1
 assert not proc.stdout.read(1)
assert proc.wait()==0 and count==245760
print('PASS all245760 complete original LM/CM fixed display source states under ASAN/UBSAN, SHA256',h.hexdigest())
