#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Real scheduler canonical exchange actions versus unchanged source CHANG2."""
import argparse,hashlib,os,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--sanitize',action='store_true');a=p.parse_args();stem='lm-interpreter-exchange-sanitize' if a.sanitize else 'lm-interpreter-exchange-native'
flags=['gcc','-std=c99','-O1' if a.sanitize else '-O2','-Wall','-Wextra','-Werror','-Iinclude','-Itests','-DP66_NATIVE_DISPATCH_ONLY','-ffunction-sections','-Wl,--gc-sections']
if a.sanitize:flags+=['-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']
subprocess.run(flags+['src/agc_word.c','src/executive.c','src/native_dispatch.c','src/irq_delivery.c','src/p66_native.c','src/lm_interpreter_change.c','tests/lm_interpreter_change_cases.c','tests/lm_interpreter_exchange_native.c','tools/lm_interpreter_exchange_snapshot.c','-o','build/'+stem],cwd=ROOT,check=True)
with (ROOT/('build/'+stem+'.snapshots')).open('w') as out:subprocess.run(['build/'+stem],cwd=ROOT,stdout=out,check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
expected=(ROOT/'build/lm-interpreter-change-reference.snapshots').read_bytes();actual=(ROOT/('build/'+stem+'.snapshots')).read_bytes();assert expected==actual
print('PASS:16384 realstep exchanges,all2052 raw original source words; no second source lifecycle or IRQpoll')
print('Snapshot SHA256:',hashlib.sha256(actual).hexdigest())
