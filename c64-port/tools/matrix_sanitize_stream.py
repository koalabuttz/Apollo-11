#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma,hashlib,os
R=pathlib.Path(__file__).resolve().parents[1];core=['src/agc_word.c','src/agc_multiword.c','src/guidance_product.c','src/matrix_native.c','tests/matrix_cases.c'];subprocess.run(['cc','-O1','-g','-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','-fsanitize=address,undefined','-fno-omit-frame-pointer']+core+['tools/matrix_snapshot.c','-o','build/matrix-sanitize'],cwd=R,check=True);env=os.environ.copy();env['ASAN_OPTIONS']='detect_leaks=0'
for mission in ['CM','LM']:
 proc=subprocess.Popen(['build/matrix-sanitize',mission],cwd=R,stdout=subprocess.PIPE,env=env);h=hashlib.sha256()
 with lzma.open(R/f'build/matrix-{mission}-reference.snapshots.xz','rb')as ref,lzma.open(R/f'build/matrix-{mission}-sanitize.snapshots.xz','wb',preset=3,check=lzma.CHECK_SHA256)as out:
  for n,expected in enumerate(ref):
   actual=proc.stdout.readline();assert actual==expected,(mission,n);h.update(actual);out.write(actual)
  assert not proc.stdout.read(1)
 assert n==32767 and proc.wait()==0
 print('PASS all32768 complete',mission,'MXV/VXM states under ASAN/UBSAN, SHA256',h.hexdigest(),flush=True)
