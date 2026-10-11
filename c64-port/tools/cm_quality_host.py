#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Strict and sanitized canonical QUALITY phases against original instructions."""
import argparse,hashlib,json,lzma,os,pathlib,subprocess
R=pathlib.Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--generate',action='store_true');p.add_argument('--sanitize',action='store_true');a=p.parse_args()
core=['tests/cm_quality_cases.c','src/cm_quality.c','src/interpreted_square.c','src/guidance_product.c','src/guidance_sum.c','src/guidance_shift.c','src/agc_word.c','src/agc_multiword.c']
flags=['gcc','-std=c99','-O1'if a.sanitize else'-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests']
if a.sanitize:flags+=['-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']
binary='build/cm-quality-'+('sanitize'if a.sanitize else'native')
subprocess.run(flags+['tools/cm_quality_snapshot.c',*core,'-o',binary],cwd=R,check=True)
archive=R/'build/cm-quality-original.states.xz'
if a.generate:
 assert not a.sanitize
 subprocess.run(['gcc','-std=c99','-O2','-Iinclude','-Itests','-Ideps/virtualagc/yaAGC','tools/cm_quality_reference.c','tests/cm_quality_cases.c','tests/cm_executive_cases.c','src/agc_word.c','build/agc_engine_hardware_dv.o','deps/virtualagc/yaAGC/agc_engine_init.o','deps/virtualagc/yaAGC/rfopen.o','-o','build/cm-quality-reference'],cwd=R,check=True)
 proc=subprocess.Popen(['build/cm-quality-reference','deps/cm-rtb-fixture/MAIN.agc.bin','0','65536'],cwd=R,stdout=subprocess.PIPE)
 h=hashlib.sha256();size=0
 with lzma.open(archive,'wb',preset=3)as out:
  while block:=proc.stdout.read(1048576):out.write(block);h.update(block);size+=len(block)
 assert proc.wait()==0 and size==688128*4108
 record={'archive':'evidence/cm-quality-original.states.xz','decoded_sha256':h.hexdigest(),'decoded_bytes':size,'states':688128,'inputs':65536}
 (R/'build/cm-quality-source-stream.json').write_text(json.dumps(record,indent=2)+'\n')
else:
 path=R/'evidence/cm-quality-source-stream.json'
 if path.exists():record=json.loads(path.read_text());archive=R/record['archive']
 else:record=json.loads((R/'build/cm-quality-source-stream.json').read_text())
env=os.environ.copy();env['ASAN_OPTIONS']='detect_leaks=0'
subprocess.run(['python3','tools/qualified_stream_host.py','--binary',binary,'--archive',str(archive),'--sha256',record['decoded_sha256'],'--bytes',str(record['decoded_bytes']),'--arg','0','--arg','65536'],cwd=R,env=env,check=True)
print('PASS688128 original QUALITY checkpoint states'+(' full ASAN/UBSAN'if a.sanitize else' strict native'))
