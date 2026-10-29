#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import argparse,pathlib,subprocess
R=pathlib.Path(__file__).resolve().parents[1];p=argparse.ArgumentParser();p.add_argument('--mos-cc',required=True);a=p.parse_args()
core=['src/agc_word.c', 'src/agc_multiword.c', 'src/fixed_data.c', 'src/noun_reader.c', 'src/guidance_product.c', 'src/dsky_scale.c', 'src/cm_scale.c', 'src/dsky_pinball.c', 'src/dsky_decimal_load.c', 'src/dsky_decimal_select.c', 'src/dsky_scalar_output.c', 'src/dsky_decimal_row.c', 'src/dsky_decimal_general.c', 'src/dsky_decimal_raw.c', 'src/dsky_decimal_consumer.c', 'tests/dsky_decimal_consumer_cases.c', 'tests/dsky_decimal_consumer_check.c']
for stage in range(7):
 subprocess.run([a.mos_cc,'-Os','-fno-static-stack','-mlto-zp=0','-ffunction-sections','-Wl,--gc-sections','-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','-DDSKY_DECIMAL_CONSUMER_ONLY_STAGE='+str(stage),'tools/dsky_decimal_consumer_diagnostic.c']+core+['-Wl,-Map,build/dsky_decimal_consumer_'+str(stage)+'.map','-o','build/dsky_decimal_consumer_'+str(stage)+'.prg'],cwd=R,check=True)
