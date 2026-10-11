#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
from pathlib import Path
import hashlib,lzma,os,subprocess
ROOT=Path(__file__).resolve().parents[1]
CORE=['src/agc_word.c','src/agc_multiword.c','src/guidance_product.c','src/guidance_shift.c','src/guidance_sum.c','src/guidance_trig.c','src/guidance_dot.c','src/guidance_dp_divide.c','src/delcomp.c','src/gimbal_stage.c','src/gimbal_complete.c','tests/delcomp_cases.c','tests/gimbal_complete_cases.c']
def run(cmd,**kw):subprocess.run(cmd,cwd=ROOT,check=True,**kw)
def main():
 run(['python3','tools/gimbal_complete_source.py'])
 common=['-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests']
 run(['cc','-O2',*common,*CORE,'tests/test_gimbal_complete.c','-o','build/test-gimbal-complete']);run(['build/test-gimbal-complete'])
 run(['cc','-O2',*common,*CORE,'tools/gimbal_complete_snapshot.c','-o','build/gimbal-complete-native'])
 for name,extra in [('native',[]),('input',['--input'])]:
  with (ROOT/f'build/gimbal-complete-{name}.snapshots').open('wb') as out:run(['build/gimbal-complete-native',*extra],stdout=out)
 objects=[str(ROOT/'deps/virtualagc/yaAGC'/n) for n in ['agc_engine.o','agc_engine_init.o','rfopen.o']]
 run(['cc','-std=c99','-O2','-Iinclude','-Itests','-Ideps/virtualagc/yaAGC','tools/gimbal_complete_reference.c','tests/delcomp_cases.c','tests/gimbal_complete_cases.c','tests/scenarios.c',*objects,'-o','build/gimbal-complete-reference'])
 for short,mission in [('lm','Luminary099'),('cm','Comanche055')]:
  with (ROOT/f'build/gimbal-complete-{short}.snapshots').open('wb') as out:run(['build/gimbal-complete-reference',f'build/gimbal-complete-source/{mission}/MAIN.agc.bin'],stdout=out)
 original=(ROOT/'build/gimbal-complete-lm.snapshots').read_bytes()+(ROOT/'build/gimbal-complete-cm.snapshots').read_bytes()
 assert len(original)==10240*4104 and (ROOT/'build/gimbal-complete-native.snapshots').read_bytes()==original
 san=['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'];env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0')
 run(['cc',*san,*common,*CORE,'tests/test_gimbal_complete.c','-o','build/test-gimbal-complete-sanitize']);run(['build/test-gimbal-complete-sanitize'],env=env)
 run(['cc',*san,*common,*CORE,'tools/gimbal_complete_snapshot.c','-o','build/gimbal-complete-sanitize'])
 with (ROOT/'build/gimbal-complete-sanitize.snapshots').open('wb') as out:run(['build/gimbal-complete-sanitize'],stdout=out,env=env)
 assert (ROOT/'build/gimbal-complete-sanitize.snapshots').read_bytes()==original
 for name in ['reference','native']:(ROOT/f'evidence/gimbal-complete-{name}.snapshots.xz').write_bytes(lzma.compress(original))
 (ROOT/'evidence/gimbal-complete-input.snapshots.xz').write_bytes(lzma.compress((ROOT/'build/gimbal-complete-input.snapshots').read_bytes()))
 print('PASS: 10240 complete original-source/native/sanitized whole gimbal states, all2048 words andSUPER/IRQ exact')
 print('Uncompressed source/native SHA256:',hashlib.sha256(original).hexdigest())
 run(['python3','tools/freeze_gimbal_complete.py'])
if __name__=='__main__':main()
