#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import argparse,pathlib,subprocess
R=pathlib.Path(__file__).resolve().parents[1];p=argparse.ArgumentParser();p.add_argument('--mos-cc',required=True);a=p.parse_args()
core=['src/agc_word.c', 'src/agc_multiword.c', 'src/fixed_data.c', 'src/noun_reader.c', 'src/guidance_product.c', 'src/dsky_pinball.c', 'src/dsky_five_blank.c', 'src/dsky_sign_output.c', 'src/dsky_read_pair.c', 'src/dsky_decimal_load.c', 'src/dsky_decimal_select.c', 'src/dsky_decimal_raw.c', 'src/dsky_hours_output.c', 'src/dsky_fraction_pair.c', 'src/dsky_minutes_output.c', 'src/dsky_two_integer.c', 'src/dsky_decimal_time_consumer.c', 'tests/dsky_decimal_time_consumer_cases.c', 'tests/dsky_decimal_time_consumer_check.c']
for stage in range(22):
 role=[]
 if 2<=stage<=9:role=["-DDSKY_HOURS_ONLY_STAGE="+str(stage-2)]
 elif 10<=stage<=15:role=["-DDSKY_MINUTES_ONLY_STAGE="+str(stage-10)]
 elif 16<=stage<=20:role=["-DDSKY_TWO_INTEGER_ONLY_STAGE="+str(stage-16)]
 subprocess.run([a.mos_cc,'-Os','-fno-static-stack','-mlto-zp=0','-ffunction-sections','-Wl,--gc-sections','-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','-DDSKY_DECIMAL_TIME_CONSUMER_ONLY_STAGE='+str(stage),'tools/dsky_decimal_time_consumer_diagnostic.c']+role+core+['-Wl,-Map,build/dsky_decimal_time_consumer_'+str(stage)+'.map','-o','build/dsky_decimal_time_consumer_'+str(stage)+'.prg'],cwd=R,check=True)
