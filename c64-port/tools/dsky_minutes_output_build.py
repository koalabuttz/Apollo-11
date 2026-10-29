#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import argparse,pathlib,subprocess
R=pathlib.Path(__file__).resolve().parents[1];p=argparse.ArgumentParser();p.add_argument('--mos-cc',required=True);a=p.parse_args()
core=['src/agc_word.c','src/agc_multiword.c','src/guidance_product.c','src/dsky_pinball.c','src/dsky_fraction_pair.c','src/dsky_minutes_output.c','tests/dsky_minutes_output_cases.c','tests/dsky_minutes_output_check.c']
for stage in range(6):
 subprocess.run([a.mos_cc,'-Os','-fno-static-stack','-mlto-zp=0','-ffunction-sections','-Wl,--gc-sections','-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','-DDSKY_MINUTES_ONLY_STAGE='+str(stage),'tools/dsky_minutes_output_diagnostic.c']+core+['-Wl,-Map,build/dsky_minutes_output_'+str(stage)+'.map','-o','build/dsky_minutes_output_'+str(stage)+'.prg'],cwd=R,check=True)
