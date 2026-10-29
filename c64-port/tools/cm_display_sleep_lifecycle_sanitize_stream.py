#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma,hashlib,os
R=pathlib.Path(__file__).resolve().parents[1]
core=['src/agc_word.c','src/cm_makeplay_sleep.c','src/cm_basic_wake.c','src/cm_basic_sleep.c','src/cm_native_image.c','src/cm_dispatch.c','src/cm_executive.c','tests/cm_trig_cases.c','tests/cm_clockjob_cases.c','tests/cm_wake_cases.c','tests/cm_display_sleep_lifecycle_cases.c']
subprocess.run(['cc','-O1','-g','-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-ffunction-sections','-Wl,--gc-sections','-Iinclude','-Itests','-fsanitize=address,undefined','-fno-omit-frame-pointer']+core+['tools/cm_display_sleep_lifecycle_snapshot.c','-o','build/cm-display-sleep-lifecycle-sanitize'],cwd=R,check=True)
env=os.environ.copy();env['ASAN_OPTIONS']='detect_leaks=0';proc=subprocess.Popen(['build/cm-display-sleep-lifecycle-sanitize'],cwd=R,stdout=subprocess.PIPE,env=env);h=hashlib.sha256()
with lzma.open(R/'build/cm-display-sleep-lifecycle-reference.snapshots.xz','rb')as ref,lzma.open(R/'build/cm-display-sleep-lifecycle-sanitize.snapshots.xz','wb',preset=3,check=lzma.CHECK_SHA256)as out:
 for n,expected in enumerate(ref):
  actual=proc.stdout.readline();assert actual==expected,n;h.update(actual);out.write(actual)
 assert not proc.stdout.read(1)
assert n==2047 and proc.wait()==0
print('PASS: ASAN/UBSAN all2048 complete original CM MAKEPLAY states, SHA256',h.hexdigest())
