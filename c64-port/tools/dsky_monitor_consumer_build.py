#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,argparse
R=pathlib.Path(__file__).resolve().parents[1];p=argparse.ArgumentParser();p.add_argument('--mos-cc',required=True);a=p.parse_args()
core=["src/agc_multiword.c", "src/agc_word.c", "src/cm_interbank_complete.c", "src/cm_scale.c", "src/dsky_blank.c", "src/dsky_blanksub.c", "src/dsky_decimal_consumer.c", "src/dsky_decimal_general.c", "src/dsky_decimal_load.c", "src/dsky_decimal_raw.c", "src/dsky_decimal_row.c", "src/dsky_decimal_select.c", "src/dsky_machine_cadr.c", "src/dsky_mixed_noun.c", "src/dsky_monitor.c", "src/dsky_monitor_consumer.c", "src/dsky_monitor_paste.c", "src/dsky_nv_update_general.c", "src/dsky_octal_consumer.c", "src/dsky_octal_display.c", "src/dsky_octal_verb.c", "src/dsky_pinball.c", "src/dsky_scalar_output.c", "src/dsky_scale.c", "src/dsky_test_noun.c", "src/dsky_verb_dispatch.c", "src/fixed_data.c", "src/guidance_product.c", "src/lm_interbank.c", "src/noun_reader.c", "tests/dsky_monitor_consumer_cases.c"]
legacy='src/dsky_machine_cadr.c'
core.remove(legacy)
subprocess.run([a.mos_cc,'-Os','-fno-static-stack','-ffunction-sections','-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-Wno-sign-conversion','-pedantic','-Iinclude','-Itests','-c',legacy,'-o','build/dsky-monitor-consumer-inherited-machine.o'],cwd=R,check=True)
core.append('build/dsky-monitor-consumer-inherited-machine.o')
for n in [0, 1, 2, 3, 4, 6, 7, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23]:
 subprocess.run([a.mos_cc,'-Os','-fno-static-stack','-mlto-zp=0','-ffunction-sections','-Wl,--gc-sections','-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','-DDSKY_MONITOR_CONSUMER_ONLY_STAGE='+str(n),'tools/dsky_monitor_consumer_diagnostic.c']+core+['tests/dsky_monitor_consumer_check.c','-Wl,-Map,build/dsky_monitor_consumer_'+str(n)+'.map','-o','build/dsky_monitor_consumer_'+str(n)+'.prg'],cwd=R,check=True)
