# SPDX-License-Identifier: MIT
DSKY_MONITOR_CONSUMER_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
DSKY_MONITOR_CONSUMER_CORE = src/agc_multiword.c src/agc_word.c src/cm_interbank_complete.c src/cm_scale.c src/dsky_blank.c src/dsky_blanksub.c src/dsky_decimal_consumer.c src/dsky_decimal_general.c src/dsky_decimal_load.c src/dsky_decimal_raw.c src/dsky_decimal_row.c src/dsky_decimal_select.c src/dsky_machine_cadr.c src/dsky_mixed_noun.c src/dsky_monitor.c src/dsky_monitor_consumer.c src/dsky_monitor_paste.c src/dsky_nv_update_general.c src/dsky_octal_consumer.c src/dsky_octal_display.c src/dsky_octal_verb.c src/dsky_pinball.c src/dsky_scalar_output.c src/dsky_scale.c src/dsky_test_noun.c src/dsky_verb_dispatch.c src/fixed_data.c src/guidance_product.c src/lm_interbank.c src/noun_reader.c tests/dsky_monitor_consumer_cases.c
all: build/test-dsky-monitor-consumer build/dsky-monitor-consumer-route-port
build/test-dsky-monitor-consumer: tests/test_dsky_monitor_consumer.c $(DSKY_MONITOR_CONSUMER_CORE) tests/dsky_monitor_consumer_check.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_MONITOR_CONSUMER_FLAGS) $^ -o $@
build/dsky-monitor-consumer-route-port: tools/dsky_monitor_consumer_snapshot.c $(DSKY_MONITOR_CONSUMER_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_MONITOR_CONSUMER_FLAGS) $^ -o $@
test: build/test-dsky-monitor-consumer
	build/test-dsky-monitor-consumer
sanitize:
	python3 tools/dsky_monitor_consumer_sanitize.py
