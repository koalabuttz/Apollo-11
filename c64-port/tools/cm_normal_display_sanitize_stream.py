#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma,hashlib,os
R=pathlib.Path(__file__).resolve().parents[1]
core=['src/agc_multiword.c', 'src/agc_word.c', 'src/cm_basic_finish.c', 'src/cm_basic_priority.c', 'src/cm_basic_wake.c', 'src/cm_clockjob.c', 'src/cm_decimal_pair.c', 'src/cm_dispatch.c', 'src/cm_display_control.c', 'src/cm_executive.c', 'src/cm_formatter.c', 'src/cm_interbank.c', 'src/cm_makeplay.c', 'src/cm_minutes_seconds.c', 'src/cm_native_image.c', 'src/cm_normal_display.c', 'src/cm_normal_display_route.c', 'src/cm_nv85.c', 'src/cm_nv85_route.c', 'src/cm_pinball.c', 'src/cm_scale.c', 'src/dsky_decimal.c', 'src/dsky_pinball.c', 'src/dsky_scale.c', 'src/dsky_two_digit.c', 'src/fixed_data.c', 'src/guidance_product.c', 'tests/cm_clockjob_cases.c', 'tests/cm_normal_display_cases.c', 'tests/cm_nv85_apply.c', 'tests/cm_nv85_cases.c', 'tests/cm_trig_cases.c']
subprocess.run(['cc','-O1','-g','-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']+core+['tools/cm_normal_display_snapshot.c','-o','build/cm-normal-display-sanitize'],cwd=R,check=True)
proc=subprocess.Popen(['build/cm-normal-display-sanitize'],cwd=R,stdout=subprocess.PIPE,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'));h=hashlib.sha256();count=0
with lzma.open(R/'build/cm-normal-display-reference.snapshots.xz','rb')as ref:
 for expected in ref:
  actual=proc.stdout.readline();assert actual==expected,count;h.update(actual);count+=1
 assert not proc.stdout.read(1)
assert proc.wait()==0 and count==18688
print('PASS all18688 complete original V06N40/N85 source states under ASAN/UBSAN, SHA256',h.hexdigest())
