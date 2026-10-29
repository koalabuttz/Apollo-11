#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Build frozen full-state source-selected native adoption target diagnostics."""
import argparse,pathlib,subprocess
root=pathlib.Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--cc',default='mos-c64-clang');a=p.parse_args()
common=[a.cc,'-Oz','-fno-static-stack','-mlto-zp=0','-std=c99','-Wall','-Wextra','-Werror','-Iinclude','-Itests','-DEXEC_NATIVE_FINISH_FAMILY=3','-DP66_NATIVE_DISPATCH_ONLY']
core=['src/native_dispatch.c','src/irq_delivery.c','src/executive.c','src/agc_word.c','src/p66_native.c','tests/lm_sleep_cases.c','tests/lm_priority_cases.c','tests/throttle_bridge_cases.c']
subprocess.run(['python3','tools/freeze_native_source_dispatch.py'],cwd=root,check=True)
for name,source in(('dispatch',['src/lm_basic_priority.c','src/lm_basic_sleep.c']),('finish',['src/lm_basic_finish.c','tests/lm_finish_cases.c'])):
 flags=['-DNATIVE_SOURCE_DISPATCH_SMALL_VECTORS','-DNATIVE_SOURCE_DISPATCH_CASES=32']if name=='dispatch'else[]
 subprocess.run(common+flags+[f'platform/c64/native_source_{name}_diagnostic.c',f'tests/native_source_{name}_check.c']+source+core+[f'-Wl,-Map,build/native-source-{name}.map','-o',f'build/native-source-{name}.prg'],cwd=root,check=True)
