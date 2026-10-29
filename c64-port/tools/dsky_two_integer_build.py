#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import argparse,pathlib,subprocess
R=pathlib.Path(__file__).resolve().parents[1];p=argparse.ArgumentParser();p.add_argument('--mos-cc',required=True);a=p.parse_args()
core=['src/agc_word.c','src/agc_multiword.c','src/dsky_pinball.c','src/dsky_five_blank.c','src/dsky_sign_output.c','src/dsky_read_pair.c','src/dsky_two_integer.c','tests/dsky_two_integer_cases.c','tests/dsky_two_integer_check.c']
for stage in range(5):
 subprocess.run([a.mos_cc,'-Os','-fno-static-stack','-mlto-zp=0','-ffunction-sections','-Wl,--gc-sections','-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','-DDSKY_TWO_INTEGER_ONLY_STAGE='+str(stage),'tools/dsky_two_integer_diagnostic.c']+core+['-Wl,-Map,build/dsky_two_integer_'+str(stage)+'.map','-o','build/dsky_two_integer_'+str(stage)+'.prg'],cwd=R,check=True)
