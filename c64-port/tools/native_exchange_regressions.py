#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Supplementary current-core targets; historical artifact names stay intact."""
import argparse,os,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--cc',default='mos-c64-clang');p.add_argument('--vice',default='x64sc');p.add_argument('--rom-dir',required=True);p.add_argument('--data-dir',required=True);a=p.parse_args();build=ROOT/'build'
def run(command):subprocess.run(command,cwd=ROOT,check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
def assignment(file,name):return next(line for line in (ROOT/file).read_text().splitlines() if line.startswith(name+' =')).split(' = ',1)[1].split()
common=['-std=c99','-Wall','-Wextra','-Werror','-Iinclude','-Itests']
for kind in ('mixed','keyboard-irq','keyboard-number'):
    stem='native-exchange-'+kind
    if kind=='mixed':
        core=assignment('Makefile','CORE');tests=['tests/interpreter_native_cases.c','tests/interpreter_native_check.c','tests/state_digest.c'];flags=[];host='tests/test_interpreter_native.c';target=['platform/c64/interpreter_native_diagnostic.c','platform/c64/irq_platform.c','platform/c64/irq_gate.S'];runner='tools/interpreter_native_vice_test.py';old='interpreter-native'
    else:
        group='IRQ' if kind=='keyboard-irq' else 'NUMBER';base='dsky_keyboard_irq_flow' if group=='IRQ' else 'dsky_keyboard_number_flow';mk='tools/'+base+'.mk';prefix='DSKY_KEYBOARD_'+group+'_FLOW';core=assignment(mk,prefix+'_CORE');tests=assignment(mk,prefix+'_TEST');flags=assignment(mk,prefix+'_FLAGS');host='tests/test_'+base+'.c';target=['platform/c64/dsky_keyboard.c','tools/'+base+'_diagnostic.c'];runner='tools/'+base+'_vice_test.py';old=base
    run(['gcc','-O2',*common,*flags,*core,*tests,host,'-o','build/'+stem+'-host']);run(['build/'+stem+'-host'])
    run(['gcc','-O1',*common,'-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie',*flags,*core,*tests,host,'-o','build/'+stem+'-san']);run(['build/'+stem+'-san'])
    extra=[] if kind=='mixed' else ['-DEXEC_NATIVE_FINISH_FAMILY=2','-D'+prefix+'_C64']
    run([a.cc,'-Os' if kind=='mixed' else '-Oz',*common,'-Iplatform/c64','-fno-static-stack','-mlto-zp=0',*extra,*flags,*core,*tests,*target,'-Wl,-Map,build/'+stem+'.map','-o','build/'+stem+'.prg'])
    text=(ROOT/runner).read_text().replace(old,stem)
    if kind!='mixed':text=text.replace(stem+'_mailbox',old+'_mailbox')
    script=build/(stem+'-vice.py');script.write_text(text)
    # The copied runner's parent root remains c64-port because build is one
    # directory below it, exactly like the original tools directory.
    run(['python3',str(script),'--vice',a.vice,'--rom-dir',a.rom_dir,'--data-dir',a.data_dir])
print('PASS:old mixed,wholeKEYIRQ/CHARIN anddigitflow host/SAN/current-coreC64 supplementarytargets')
