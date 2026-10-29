#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Rebuild byte-original source and qualify the native full-image routine."""
from pathlib import Path
import argparse,hashlib,lzma,os,shutil,subprocess
ROOT=Path(__file__).resolve().parents[1]
def run(cmd,**kw):subprocess.run(cmd,cwd=ROOT,check=True,**kw)
def main():
 p=argparse.ArgumentParser();p.add_argument('--cc',default='cc');args=p.parse_args()
 work=ROOT/'build/lm-torque-source';work.mkdir(exist_ok=True)
 for f in (ROOT/'build/fixed-data/Luminary099').glob('*.agc'):shutil.copyfile(f,work/f.name)
 source=ROOT.parent/'Luminary099/KALMAN_FILTER.agc';shutil.copyfile(source,work/source.name)
 with (work/'assembly.lst').open('w') as out:subprocess.run([str(ROOT/'deps/virtualagc/yaYUL/yaYUL'),'MAIN.agc'],cwd=work,stdout=out,stderr=subprocess.STDOUT,check=True)
 assert (work/'MAIN.agc.bin').read_bytes()==(ROOT/'build/fixed-data/Luminary099/MAIN.agc.bin').read_bytes()
 common=['-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests']
 native=['src/agc_word.c','src/agc_multiword.c','src/lm_torque.c','tests/lm_torque_cases.c']
 run([args.cc,'-O2',*common,*native,'tests/test_lm_torque.c','-o','build/test-lm-torque']);run(['build/test-lm-torque'])
 run([args.cc,'-O2',*common,*native,'tools/lm_torque_snapshot.c','-o','build/lm-torque-native'])
 for suffix,extra in [('native',[]),('input',['--input'])]:
  with (ROOT/f'build/lm-torque-{suffix}.snapshots').open('wb') as out:run(['build/lm-torque-native',*extra],stdout=out)
 objects=[str(ROOT/'deps/virtualagc/yaAGC'/n) for n in ['agc_engine.o','agc_engine_init.o','rfopen.o']]
 run([args.cc,'-std=c99','-O2','-Iinclude','-Itests','-Ideps/virtualagc/yaAGC','tools/lm_torque_reference.c','tests/lm_torque_cases.c','tests/scenarios.c',*objects,'-o','build/lm-torque-reference'])
 with (ROOT/'build/lm-torque-reference.snapshots').open('wb') as out:run(['build/lm-torque-reference',str(work/'MAIN.agc.bin')],stdout=out)
 original=(ROOT/'build/lm-torque-reference.snapshots').read_bytes()
 actual=(ROOT/'build/lm-torque-native.snapshots').read_bytes()
 assert len(original)==36864*4104
 if original!=actual:
  i=next(i for i,(a,b) in enumerate(zip(original,actual)) if a!=b)
  raise AssertionError(('full state mismatch',i//4104,i%4104//2))
 san=['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer']
 env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0')
 run([args.cc,*san,*common,*native,'tests/test_lm_torque.c','-o','build/test-lm-torque-sanitize']);run(['build/test-lm-torque-sanitize'],env=env)
 run([args.cc,*san,*common,*native,'tools/lm_torque_snapshot.c','-o','build/lm-torque-sanitize'])
 with (ROOT/'build/lm-torque-sanitize.snapshots').open('wb') as out:run(['build/lm-torque-sanitize'],stdout=out,env=env)
 assert (ROOT/'build/lm-torque-sanitize.snapshots').read_bytes()==original
 for name,data in [('reference',original),('native',actual)]:
  (ROOT/f'evidence/lm-torque-{name}.snapshots.xz').write_bytes(lzma.compress(data))
 print('PASS: 36864 complete original-source/native/sanitized images; all 2048 words and external latches match')
 print('Uncompressed source/native SHA256:',hashlib.sha256(original).hexdigest())
 run(['python3','tools/freeze_lm_torque.py']);run(['python3','tools/lm_torque_audit.py'])
if __name__=='__main__':main()
