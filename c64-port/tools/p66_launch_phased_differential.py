#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Original raw source state at the three compiled startup call seams."""
import argparse, hashlib, os, subprocess
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
p = argparse.ArgumentParser(); p.add_argument('--sanitize', action='store_true'); a = p.parse_args()
core = next(line for line in (ROOT/'tools/p66_launch.mk').read_text().splitlines() if line.startswith('P66_LAUNCH_CORE =')).split(' = ')[1].split()
factory = ['tests/p66_bias_cases.c','tests/p66_vertical_start_cases.c','tests/p66_launch_cases.c']
flags = ['gcc','-std=c99','-O1' if a.sanitize else '-O2','-Wall','-Wextra','-Werror','-Iinclude','-Itests']
if a.sanitize: flags += ['-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']
stem = 'p66-launch-phased-sanitize' if a.sanitize else 'p66-launch-phased-native'
subprocess.run(flags+core+factory+['src/p66_launch_phased.c','tools/p66_launch_phased_snapshot.c','-o','build/'+stem],cwd=ROOT,check=True)
if not a.sanitize:
    subprocess.run(['python3','tools/prepare_hardware_dv.py'],cwd=ROOT,check=True)
    engine=ROOT/'deps/virtualagc/yaAGC'
    subprocess.run(flags+['-I'+str(engine),'tools/p66_launch_phased_reference.c','tests/scenarios.c','src/agc_word.c']+factory+['build/agc_engine_hardware_dv.o',str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/p66-launch-phased-reference'],cwd=ROOT,check=True)
    with (ROOT/'build/p66-launch-phased-reference.snapshots').open('w') as out:
        subprocess.run(['build/p66-launch-phased-reference','deps/fixture/MAIN.agc.bin'],cwd=ROOT,stdout=out,check=True)
with (ROOT/('build/'+stem+'.snapshots')).open('w') as out:
    subprocess.run(['build/'+stem],cwd=ROOT,stdout=out,check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
reference=(ROOT/'build/p66-launch-phased-reference.snapshots').read_bytes(); native=(ROOT/('build/'+stem+'.snapshots')).read_bytes()
if reference!=native:
    for expected,actual in zip(reference.splitlines(),native.splitlines()):
        if expected!=actual:
            x=list(map(int,expected.split())); y=list(map(int,actual.split()));raise SystemExit((x[:3],[(i-3,left,right) for i,(left,right) in enumerate(zip(x,y)) if left!=right][:20]))
    raise SystemExit('unequal stream length')
print('PASS:',len(native.splitlines()),'complete2050-word startup source seams')
print('Snapshot SHA256:',hashlib.sha256(native).hexdigest())
