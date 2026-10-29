#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import argparse,pathlib,subprocess
R=pathlib.Path(__file__).resolve().parents[1];p=argparse.ArgumentParser();p.add_argument('--mos-cc',required=True);a=p.parse_args()
core=['src/agc_word.c','src/dsky_monitor.c','tests/dsky_monitor_cases.c','tests/dsky_monitor_check.c']
for stage in range(11):
 subprocess.run([a.mos_cc,'-Os','-fno-static-stack','-mlto-zp=0','-ffunction-sections','-Wl,--gc-sections','-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','-DDSKY_MONITOR_ONLY_STAGE='+str(stage),'tools/dsky_monitor_diagnostic.c']+core+['-Wl,-Map,build/dsky_monitor_'+str(stage)+'.map','-o','build/dsky_monitor_'+str(stage)+'.prg'],cwd=R,check=True)
