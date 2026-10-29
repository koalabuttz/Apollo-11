#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
from pathlib import Path
import hashlib,lzma,os,subprocess
ROOT=Path(__file__).resolve().parents[1]
CORE=['src/agc_word.c','src/agc_multiword.c','src/guidance_product.c','src/guidance_shift.c','src/guidance_sum.c','src/guidance_trig.c','src/delcomp.c','tests/delcomp_cases.c']
def run(cmd,**kw):subprocess.run(cmd,cwd=ROOT,check=True,**kw)
def main():
 run(['python3','tools/delcomp_source.py'])
 common=['-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests']
 run(['cc','-O2',*common,*CORE,'tests/test_delcomp.c','-o','build/test-delcomp']);run(['build/test-delcomp'])
 run(['cc','-O2',*common,*CORE,'tools/delcomp_snapshot.c','-o','build/delcomp-native'])
 for name,extra in [('native',[]),('input',['--input'])]:
  with (ROOT/f'build/delcomp-{name}.snapshots').open('wb') as out:run(['build/delcomp-native',*extra],stdout=out)
 objects=[str(ROOT/'deps/virtualagc/yaAGC'/n) for n in ['agc_engine.o','agc_engine_init.o','rfopen.o']]
 run(['cc','-std=c99','-O2','-Iinclude','-Itests','-Ideps/virtualagc/yaAGC','tools/delcomp_reference.c','tests/delcomp_cases.c','tests/scenarios.c',*objects,'-o','build/delcomp-reference'])
 for short,mission in [('lm','Luminary099'),('cm','Comanche055')]:
  with (ROOT/f'build/delcomp-{short}.snapshots').open('wb') as out:run(['build/delcomp-reference',f'build/delcomp-source/{mission}/MAIN.agc.bin'],stdout=out)
 original=(ROOT/'build/delcomp-lm.snapshots').read_bytes()+(ROOT/'build/delcomp-cm.snapshots').read_bytes()
 assert len(original)==10240*4104 and (ROOT/'build/delcomp-native.snapshots').read_bytes()==original
 san=['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'];env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0')
 run(['cc',*san,*common,*CORE,'tests/test_delcomp.c','-o','build/test-delcomp-sanitize']);run(['build/test-delcomp-sanitize'],env=env)
 run(['cc',*san,*common,*CORE,'tools/delcomp_snapshot.c','-o','build/delcomp-sanitize'])
 with (ROOT/'build/delcomp-sanitize.snapshots').open('wb') as out:run(['build/delcomp-sanitize'],stdout=out,env=env)
 assert (ROOT/'build/delcomp-sanitize.snapshots').read_bytes()==original
 for name in ['reference','native']:(ROOT/f'evidence/delcomp-{name}.snapshots.xz').write_bytes(lzma.compress(original))
 print('PASS: 10240 complete original-source/native/sanitized DELCOMP states, all2048 words andSUPER/IRQ exact')
 print('Uncompressed source/native SHA256:',hashlib.sha256(original).hexdigest())
 run(['python3','tools/freeze_delcomp.py'])
if __name__=='__main__':main()
