# SPDX-License-Identifier: MIT
INTEGRATED_DSKY_MONITOR_CONSUMER_CORE = src/agc_multiword.c src/agc_word.c src/cm_interbank_complete.c src/cm_scale.c src/dsky_blank.c src/dsky_blanksub.c src/dsky_decimal_consumer.c src/dsky_decimal_general.c src/dsky_decimal_load.c src/dsky_decimal_raw.c src/dsky_decimal_row.c src/dsky_decimal_select.c src/dsky_machine_cadr.c src/dsky_mixed_noun.c src/dsky_monitor.c src/dsky_monitor_consumer.c src/dsky_monitor_paste.c src/dsky_nv_update_general.c src/dsky_octal_consumer.c src/dsky_octal_display.c src/dsky_octal_verb.c src/dsky_pinball.c src/dsky_scalar_output.c src/dsky_scale.c src/dsky_test_noun.c src/dsky_verb_dispatch.c src/fixed_data.c src/guidance_product.c src/lm_interbank.c src/noun_reader.c tests/dsky_monitor_consumer_cases.c
INTEGRATED_DSKY_MONITOR_CONSUMER_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
all: build/test-dsky-monitor-consumer-integrated build/dsky-monitor-consumer-integrated-stream
build/test-dsky-monitor-consumer-integrated: tests/test_dsky_monitor_consumer.c $(INTEGRATED_DSKY_MONITOR_CONSUMER_CORE) tests/dsky_monitor_consumer_check.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_MONITOR_CONSUMER_FLAGS) $^ -o $@
build/dsky-monitor-consumer-integrated-stream: tools/dsky_monitor_consumer_snapshot.c $(INTEGRATED_DSKY_MONITOR_CONSUMER_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_MONITOR_CONSUMER_FLAGS) $^ -o $@
test: test-dsky-monitor-consumer-integrated
test-dsky-monitor-consumer-integrated: build/test-dsky-monitor-consumer-integrated build/dsky-monitor-consumer-integrated-stream
	./build/test-dsky-monitor-consumer-integrated
	python3 tools/qualified_stream_host.py --archive evidence/dsky-monitor-consumer-reference.snapshots.xz --sha256 b331d71beb604886d1d336c7c61221df27e6c3583b5382885f0ceb25e6f742e0 --binary build/dsky-monitor-consumer-integrated-stream
sanitize: sanitize-dsky-monitor-consumer-integrated
sanitize-dsky-monitor-consumer-integrated: | build
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_MONITOR_CONSUMER_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_dsky_monitor_consumer.c $(INTEGRATED_DSKY_MONITOR_CONSUMER_CORE) tests/dsky_monitor_consumer_check.c -o build/test-dsky-monitor-consumer-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-monitor-consumer-integrated-san
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_MONITOR_CONSUMER_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/dsky_monitor_consumer_snapshot.c $(INTEGRATED_DSKY_MONITOR_CONSUMER_CORE) -o build/dsky-monitor-consumer-integrated-stream-san
	python3 tools/qualified_stream_host.py --archive evidence/dsky-monitor-consumer-reference.snapshots.xz --sha256 b331d71beb604886d1d336c7c61221df27e6c3583b5382885f0ceb25e6f742e0 --binary build/dsky-monitor-consumer-integrated-stream-san
.PHONY: test-dsky-monitor-consumer-integrated sanitize-dsky-monitor-consumer-integrated
