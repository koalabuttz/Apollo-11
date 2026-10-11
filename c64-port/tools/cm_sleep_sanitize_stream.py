#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma,hashlib,os
R=pathlib.Path(__file__).resolve().parents[1]
core=['src/agc_word.c','src/cm_basic_sleep.c','src/cm_native_image.c','src/cm_dispatch.c','src/cm_executive.c','tests/cm_basic_finish_cases.c','tests/cm_sleep_cases.c']
subprocess.run(['cc','-O1','-g','-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-ffunction-sections','-Wl,--gc-sections','-Iinclude','-Itests','-fsanitize=address,undefined','-fno-omit-frame-pointer']+core+['tools/cm_sleep_snapshot.c','-o','build/cm-sleep-sanitize'],cwd=R,check=True)
env=os.environ.copy();env['ASAN_OPTIONS']='detect_leaks=0';proc=subprocess.Popen(['build/cm-sleep-sanitize'],cwd=R,stdout=subprocess.PIPE,env=env);h=hashlib.sha256()
with lzma.open(R/'build/cm-sleep-reference.snapshots.xz','rb')as ref,lzma.open(R/'build/cm-sleep-sanitize.snapshots.xz','wb',preset=3,check=lzma.CHECK_SHA256)as out:
 for n,expected in enumerate(ref):
  actual=proc.stdout.readline();assert actual==expected,n;h.update(actual);out.write(actual)
 assert not proc.stdout.read(1)
assert n==16383 and proc.wait()==0
print('PASS: ASAN/UBSAN all16384 complete original CM JOBSLEEP states, SHA256',h.hexdigest())
