#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Default linked source action tests: exact images, queue ownership and guards."""
import argparse,pathlib,subprocess,os
root=pathlib.Path(__file__).resolve().parents[1];p=argparse.ArgumentParser();p.add_argument('--sanitize',action='store_true');a=p.parse_args()
flags=['gcc','-O1'if a.sanitize else'-O2','-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','-DP66_NATIVE_DISPATCH_ONLY']
if a.sanitize:flags+=['-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']
core=['src/native_dispatch.c','src/irq_delivery.c','src/executive.c','src/agc_word.c','src/p66_native.c','tests/lm_sleep_cases.c','tests/lm_priority_cases.c','tests/throttle_bridge_cases.c']
for name,sources in(('dispatch',['src/lm_basic_priority.c','src/lm_basic_sleep.c']),('finish',['src/lm_basic_finish.c','tests/lm_finish_cases.c'])):
 binary=f'build/test-native-source-{name}-'+('sanitize'if a.sanitize else'native')
 subprocess.run(flags+[f'tests/test_native_source_{name}.c',f'tests/native_source_{name}_check.c']+sources+core+['-o',binary],cwd=root,check=True)
 subprocess.run([binary],cwd=root,check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
