#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import argparse,pathlib,subprocess
R=pathlib.Path(__file__).resolve().parents[1];p=argparse.ArgumentParser();p.add_argument('--mos-cc',required=True);a=p.parse_args()
core=['src/agc_word.c','src/dsky_fixed_display.c','src/lm_interbank.c','src/dsky_octal_display.c','src/dsky_pinball.c','tests/dsky_fixed_display_cases.c','tests/dsky_fixed_display_check.c']
# Frozen CM caller uses target enum comparison promotion; suppress only that
# inherited unit warning, without changing its immutable source bytes.
subprocess.run([a.mos_cc,'-Os','-fno-static-stack','-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-Wno-sign-conversion','-pedantic','-Iinclude','-c','src/cm_fixed_callers.c','-o','build/dsky-fixed-display-cm-caller.o'],cwd=R,check=True)
for stage in range(3):
 subprocess.run([a.mos_cc,'-Os','-fno-static-stack','-mlto-zp=0','-ffunction-sections','-Wl,--gc-sections','-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','-DDSKY_FIXED_DISPLAY_ONLY_STAGE='+str(stage),'tools/dsky_fixed_display_diagnostic.c']+core+['build/dsky-fixed-display-cm-caller.o']+['-Wl,-Map,build/dsky_fixed_display_'+str(stage)+'.map','-o','build/dsky_fixed_display_'+str(stage)+'.prg'],cwd=R,check=True)
