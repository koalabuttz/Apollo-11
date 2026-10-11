#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,argparse
R=pathlib.Path(__file__).resolve().parents[1];p=argparse.ArgumentParser();p.add_argument('--mos-cc',required=True);a=p.parse_args()
for n in range(3):
 subprocess.run([a.mos_cc,'-Os','-fno-static-stack','-mlto-zp=0','-ffunction-sections','-Wl,--gc-sections','-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','-DDSKY_MONITOR_PASTE_ONLY_STAGE='+str(n),'tools/dsky_monitor_paste_diagnostic.c','src/agc_word.c','src/dsky_monitor_paste.c','src/lm_interbank.c','src/cm_interbank_complete.c','tests/dsky_monitor_paste_cases.c','tests/dsky_monitor_paste_check.c','-Wl,-Map,build/dsky_monitor_paste_'+str(n)+'.map','-o','build/dsky_monitor_paste_'+str(n)+'.prg'],cwd=R,check=True)
