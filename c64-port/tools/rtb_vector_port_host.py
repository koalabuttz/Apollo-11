#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Replay all10 retained original RTB vector/frontier source corpora."""
import argparse,json,pathlib,shlex,subprocess,os
R=pathlib.Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--cc',default='cc');p.add_argument('--sanitize',action='store_true');a=p.parse_args()
proof=json.loads((R/'evidence/rtb-vector-proof.json').read_text())
core=['src/rtb_vector.c','src/agc_word.c','src/agc_multiword.c','tests/rtb_vector_cases.c']
flags=shlex.split(a.cc)+['-std=c99','-O1'if a.sanitize else'-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','-ffunction-sections','-Wl,--gc-sections']
if a.sanitize:flags+=['-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']
tag='-san'if a.sanitize else'';binary='build/rtb-vector-integrated-stream'+tag
subprocess.run(flags+['tools/rtb_vector_snapshot.c']+core+['-o',binary],cwd=R,check=True)
for name,streams in proof['logical_complete_streams'].items():
 mission=0 if name.startswith('lm')else 1;entry=int(name[2:]);state=streams['original_assembly'];frozen='build/test-rtb-vector-integrated-'+name+tag
 subprocess.run(flags+['-DVECTOR_MISSION='+str(mission),'-DVECTOR_ENTRY='+str(entry),'tests/test_rtb_vector.c','tests/frozen_words.c']+core+['-o',frozen],cwd=R,check=True)
 subprocess.run([str(R/frozen)],cwd=R,check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
 subprocess.run(['python3','tools/qualified_stream_host.py','--binary',binary,'--archive',state['object'],'--sha256',state['decoded_sha256'],'--bytes',str(state['decoded_bytes']),'--arg',str(mission),'--arg',str(entry)],cwd=R,check=True)
print('PASS all655360 complete RTB source states and320 frozen states'+(' under full ASAN/UBSAN'if a.sanitize else''),flush=True)
