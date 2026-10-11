#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma,hashlib,os
R=pathlib.Path(__file__).resolve().parents[1]
core=['src/cm_nv85.c','src/cm_nv85_route.c','src/cm_formatter.c','src/cm_pinball.c','src/dsky_two_digit.c','src/dsky_decimal.c','src/dsky_pinball.c','src/cm_scale.c','src/dsky_scale.c','src/guidance_product.c','src/agc_word.c','src/agc_multiword.c','src/cm_clockjob.c','src/cm_display_control.c','src/cm_makeplay.c','src/cm_basic_priority.c','src/cm_basic_wake.c','src/cm_basic_finish.c','src/cm_native_image.c','src/cm_executive.c','src/cm_dispatch.c','tests/cm_trig_cases.c','tests/cm_clockjob_cases.c','tests/cm_nv85_cases.c','tests/cm_nv85_apply.c']
subprocess.run(['cc','-O1','-g','-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']+core+['tools/cm_nv85_snapshot.c','-o','build/cm-nv85-sanitize'],cwd=R,check=True)
proc=subprocess.Popen(['build/cm-nv85-sanitize'],cwd=R,stdout=subprocess.PIPE,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'));h=hashlib.sha256();count=0
with lzma.open(R/'build/cm-nv85-reference.snapshots.xz','rb')as ref:
 for expected in ref:
  actual=proc.stdout.readline();assert actual==expected,count;h.update(actual);count+=1
 assert not proc.stdout.read(1)
assert proc.wait()==0 and count==13312
print('PASS all13312 complete original V06N85 source states under ASAN/UBSAN, SHA256',h.hexdigest())
