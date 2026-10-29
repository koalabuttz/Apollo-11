#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Replay all6 retained original RTB IMUPULSE service source corpora."""
import argparse,json,pathlib,shlex,subprocess,os
R=pathlib.Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--cc',default='cc');p.add_argument('--sanitize',action='store_true');a=p.parse_args()
proof=json.loads((R/'evidence/imu-pulse-proof.json').read_text())
core=['src/imu_pulse.c','src/imu_pulse_service.c','src/lm_waitlist_image.c','src/waitlist.c','src/cm_waitlist_image.c','src/cm_waitlist.c','src/cm_native_image.c','src/agc_multiword.c','src/agc_word.c','tests/imu_pulse_cases.c']
flags=shlex.split(a.cc)+['-std=c99','-O1'if a.sanitize else'-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','-ffunction-sections','-Wl,--gc-sections']
if a.sanitize:flags+=['-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']
tag='-san'if a.sanitize else'';binary='build/imu-pulse-integrated-stream'+tag
subprocess.run(flags+['tools/imu_pulse_snapshot.c']+core+['-o',binary],cwd=R,check=True)
for name,streams in proof['logical_complete_streams'].items():
 mission=0 if name.startswith('lm')else 1;entry=int(name[2:]);state=streams['original_assembly'];frozen='build/test-imu-pulse-integrated-'+name+tag
 subprocess.run(flags+['-DIMUP_MISSION='+str(mission),'-DIMUP_STAGE='+str(entry),'tests/test_imu_pulse.c','tests/frozen_words.c']+core+['-o',frozen],cwd=R,check=True)
 subprocess.run([str(R/frozen)],cwd=R,check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
 subprocess.run(['python3','tools/qualified_stream_host.py','--binary',binary,'--archive',state['object'],'--sha256',state['decoded_sha256'],'--bytes',str(state['decoded_bytes']),'--arg',str(mission),'--arg','65536','--arg',str(entry)],cwd=R,check=True)
print('PASS all393216 complete RTB source states and192 frozen states'+(' under full ASAN/UBSAN'if a.sanitize else''),flush=True)
