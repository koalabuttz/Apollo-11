#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import argparse,pathlib,subprocess
R=pathlib.Path(__file__).resolve().parents[1];p=argparse.ArgumentParser();p.add_argument('--mos-cc',required=True);a=p.parse_args()
core=['src/agc_word.c', 'src/agc_multiword.c', 'src/dsky_pinball.c', 'src/dsky_octal_display.c', 'src/dsky_octal_verb.c', 'src/dsky_octal_consumer.c', 'tests/dsky_octal_verb_cases.c', 'tests/dsky_octal_consumer_cases.c', 'tests/dsky_octal_consumer_check.c']
for stage in range(4):
 subprocess.run([a.mos_cc,'-Os','-fno-static-stack','-mlto-zp=0','-ffunction-sections','-Wl,--gc-sections','-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','-DDSKY_OCTAL_CONSUMER_ONLY_STAGE='+str(stage),'tools/dsky_octal_consumer_diagnostic.c']+core+['-Wl,-Map,build/dsky_octal_consumer_'+str(stage)+'.map','-o','build/dsky_octal_consumer_'+str(stage)+'.prg'],cwd=R,check=True)
