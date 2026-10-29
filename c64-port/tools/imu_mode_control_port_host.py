#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import argparse,json,pathlib,shlex,subprocess,os
R=pathlib.Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--cc',default='cc');p.add_argument('--sanitize',action='store_true');a=p.parse_args()
proof=json.loads((R/'evidence/imu-mode-control-proof.json').read_text())
core=['src/imu_mode_control.c', 'src/imu_coarse.c', 'src/agc_word.c', 'src/agc_multiword.c', 'tests/imu_mode_control_cases.c', 'tests/imu_coarse_cases.c', 'tests/imu_pulse_cases.c']
flags=shlex.split(a.cc)+['-std=c99','-O1'if a.sanitize else'-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','-ffunction-sections','-Wl,--gc-sections']
if a.sanitize:flags+=['-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']
tag='-san'if a.sanitize else'';binary='build/imu-mode-control-integrated-stream'+tag
subprocess.run(flags+['tools/imu_mode_control_snapshot.c']+core+['-o',binary],cwd=R,check=True)
for name,streams in proof['logical_complete_streams'].items():
 mission=0 if name.startswith('lm')else 1;stage=int(name[2:])if len(name)>2 else None;state=streams['original_assembly'];frozen='build/test-imu-mode-control-integrated-'+name+tag
 defs=['-DIMUM_MISSION='+str(mission)]+(['-DIMUM_ENTRY='+str(stage)]if stage is not None else[])
 subprocess.run(flags+defs+['tests/test_imu_mode_control.c','tests/frozen_words.c']+core+['-o',frozen],cwd=R,check=True)
 subprocess.run([str(R/frozen)],cwd=R,check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
 args=['--arg',str(mission)]+(['--arg',str(stage)]if stage is not None else[])+['--arg','65536']
 subprocess.run(['python3','tools/qualified_stream_host.py','--binary',binary,'--archive',state['object'],'--sha256',state['decoded_sha256'],'--bytes',str(state['decoded_bytes'])]+args,cwd=R,check=True)
print('PASS full retained original source states and frozen states'+(' under full ASAN/UBSAN'if a.sanitize else''),flush=True)
