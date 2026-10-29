#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma,hashlib,os
R=pathlib.Path(__file__).resolve().parents[1]
core=['src/agc_word.c','src/agc_multiword.c','src/guidance_product.c','src/guidance_shift.c','src/cm_dyndisp.c','src/cm_dyndisp_route.c','src/matrix_native.c','src/gimbal_stage.c','src/cm_powered_interface.c','src/cm_powered_axis.c','src/cm_powered_trig.c','src/cm_rotation.c','src/guidance_trig.c','tests/matrix_cases.c','tests/cm_dyndisp_cases.c','tests/cm_dyndisp_apply.c']
subprocess.run(['cc','-O1','-g','-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']+core+['tools/cm_dyndisp_snapshot.c','-o','build/cm-dyndisp-sanitize'],cwd=R,check=True)
proc=subprocess.Popen(['build/cm-dyndisp-sanitize'],cwd=R,stdout=subprocess.PIPE,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'));h=hashlib.sha256();count=0
with lzma.open(R/'build/cm-dyndisp-reference.snapshots.xz','rb')as ref:
 for expected in ref:
  actual=proc.stdout.readline();assert actual==expected,count;h.update(actual);count+=1
 assert not proc.stdout.read(1)
assert proc.wait()==0 and count==13824
print('PASS all13824 complete original DYNDISP source states under ASAN/UBSAN, SHA256',h.hexdigest())
