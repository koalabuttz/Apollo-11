#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import pathlib,subprocess,lzma,hashlib,os
R=pathlib.Path(__file__).resolve().parents[1]
core=["src/agc_multiword.c", "src/agc_word.c", "src/cm_interbank_complete.c", "src/cm_scale.c", "src/dsky_blank.c", "src/dsky_blanksub.c", "src/dsky_decimal_consumer.c", "src/dsky_decimal_general.c", "src/dsky_decimal_load.c", "src/dsky_decimal_raw.c", "src/dsky_decimal_row.c", "src/dsky_decimal_select.c", "src/dsky_machine_cadr.c", "src/dsky_mixed_noun.c", "src/dsky_monitor.c", "src/dsky_monitor_consumer.c", "src/dsky_monitor_paste.c", "src/dsky_nv_update_general.c", "src/dsky_octal_consumer.c", "src/dsky_octal_display.c", "src/dsky_octal_verb.c", "src/dsky_pinball.c", "src/dsky_scalar_output.c", "src/dsky_scale.c", "src/dsky_test_noun.c", "src/dsky_verb_dispatch.c", "src/fixed_data.c", "src/guidance_product.c", "src/lm_interbank.c", "src/noun_reader.c", "tests/dsky_monitor_consumer_cases.c"]
subprocess.run(['cc','-O1','-g','-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']+core+['tools/dsky_monitor_consumer_snapshot.c','-o','build/dsky-monitor-consumer-sanitize'],cwd=R,check=True)
proc=subprocess.Popen(['build/dsky-monitor-consumer-sanitize'],cwd=R,stdout=subprocess.PIPE,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'));h=hashlib.sha256();count=0
with lzma.open(R/'build/dsky-monitor-consumer-reference.snapshots.xz','rb')as ref:
 for expected in ref:
  actual=proc.stdout.readline();assert actual==expected,count;h.update(actual);count+=1
 assert not proc.stdout.read(1)
assert proc.wait()==0 and count==4872
print('PASS all4872 continuous complete original LM/CM monitor consumer source states under ASAN/UBSAN, SHA256',h.hexdigest())
