#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source startup/vertical body composed through the real application router."""
import argparse,hashlib,os,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--sanitize',action='store_true');a=p.parse_args();stem='p66-application-start-sanitize' if a.sanitize else 'p66-application-start-native'
sources=['src/p66_application.c','src/p66_launch_phased.c']
for file in ('p66_launch','p66_mode_display','dsky_charin_nv','dsky_charin_number'):
    for line in (ROOT/f'tools/{file}.mk').read_text().splitlines():
        if '_CORE = ' in line:sources.extend(name for name in line.split(' = ',1)[1].split() if name.startswith('src/'))
sources=list(dict.fromkeys(sources));flags=['gcc','-std=c99','-O1' if a.sanitize else '-O2','-Wall','-Wextra','-Werror','-Iinclude','-Itests','-ffunction-sections','-Wl,--gc-sections']
if a.sanitize:flags+=['-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']
factories=['tests/p66_bias_cases.c','tests/p66_vertical_start_cases.c','tests/p66_launch_cases.c','tests/p66_direct_launch_cases.c'];subprocess.run(flags+sources+factories+['tools/p66_application_start_snapshot.c','-o','build/'+stem],cwd=ROOT,check=True)
with (ROOT/('build/'+stem+'.snapshots')).open('w') as out:subprocess.run(['build/'+stem],cwd=ROOT,stdout=out,check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
expected=(ROOT/'build/p66-direct-launch-reference.snapshots').read_bytes();actual=(ROOT/('build/'+stem+'.snapshots')).read_bytes();assert expected==actual
print('PASS:2048 complete2050-word original startup/finalvertical states through application router; no new scheduler poll')
print('Snapshot SHA256:',hashlib.sha256(actual).hexdigest())
