#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
from pathlib import Path
import argparse,hashlib,lzma,os,subprocess
ROOT=Path(__file__).resolve().parents[1]
def run(cmd,**kw):subprocess.run(cmd,cwd=ROOT,check=True,**kw)
def main():
 p=argparse.ArgumentParser();p.add_argument('--cc',default='cc');args=p.parse_args()
 run(['python3','tools/gimbal_source.py'])
 common=['-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests']
 native=['src/agc_word.c','src/agc_multiword.c','src/guidance_shift.c','src/gimbal_stage.c','tests/gimbal_cases.c']
 run([args.cc,'-O2',*common,*native,'tests/test_gimbal.c','-o','build/test-gimbal']);run(['build/test-gimbal'])
 run([args.cc,'-O2',*common,*native,'tools/gimbal_snapshot.c','-o','build/gimbal-native'])
 for name,extra in [('native',[]),('input',['--input'])]:
  with (ROOT/f'build/gimbal-{name}.snapshots').open('wb') as out:run(['build/gimbal-native',*extra],stdout=out)
 objects=[str(ROOT/'deps/virtualagc/yaAGC'/n) for n in ['agc_engine.o','agc_engine_init.o','rfopen.o']]
 run([args.cc,'-std=c99','-O2','-Iinclude','-Itests','-Ideps/virtualagc/yaAGC','tools/gimbal_reference.c','tests/gimbal_cases.c','tests/scenarios.c',*objects,'-o','build/gimbal-reference'])
 for short,mission in [('lm','Luminary099'),('cm','Comanche055')]:
  with (ROOT/f'build/gimbal-{short}.snapshots').open('wb') as out:run(['build/gimbal-reference',f'build/gimbal-source/{mission}/MAIN.agc.bin'],stdout=out)
 original=(ROOT/'build/gimbal-lm.snapshots').read_bytes()+(ROOT/'build/gimbal-cm.snapshots').read_bytes()
 assert len(original)==2560*4104 and (ROOT/'build/gimbal-native.snapshots').read_bytes()==original
 san=['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'];env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0')
 run([args.cc,*san,*common,*native,'tests/test_gimbal.c','-o','build/test-gimbal-sanitize']);run(['build/test-gimbal-sanitize'],env=env)
 run([args.cc,*san,*common,*native,'tools/gimbal_snapshot.c','-o','build/gimbal-sanitize'])
 with (ROOT/'build/gimbal-sanitize.snapshots').open('wb') as out:run(['build/gimbal-sanitize'],stdout=out,env=env)
 assert (ROOT/'build/gimbal-sanitize.snapshots').read_bytes()==original
 for name in ['reference','native']:(ROOT/f'evidence/gimbal-{name}.snapshots.xz').write_bytes(lzma.compress(original))
 print('PASS: 2560 complete original-source/native/sanitized canonical entry states, all2048 words andSUPER/IRQ exact')
 print('Uncompressed source/native SHA256:',hashlib.sha256(original).hexdigest())
 run(['python3','tools/freeze_gimbal.py'])
if __name__=='__main__':main()
