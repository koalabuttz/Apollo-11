#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,argparse
R=pathlib.Path(__file__).resolve().parents[1];p=argparse.ArgumentParser();p.add_argument('--mos-cc',required=True);a=p.parse_args()
for n in range(4):
 subprocess.run([a.mos_cc,'-Os','-fno-static-stack','-mlto-zp=0','-ffunction-sections','-Wl,--gc-sections','-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','-DDSKY_TEST_NOUN_COHORT='+str(n),'tools/dsky_test_noun_diagnostic.c','src/agc_word.c','src/dsky_test_noun.c','src/noun_reader.c','src/fixed_data.c','tests/dsky_test_noun_cases.c','tests/dsky_test_noun_check.c','-Wl,-Map,build/dsky_test_noun_'+str(n)+'.map','-o','build/dsky_test_noun_'+str(n)+'.prg'],cwd=R,check=True)
