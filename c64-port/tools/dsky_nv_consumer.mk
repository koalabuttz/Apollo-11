# SPDX-License-Identifier: MIT
DSKY_NV_CONSUMER_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
DSKY_NV_CONSUMER_CORE = src/agc_word.c src/dsky_nv_consumer.c src/dsky_nv_admission.c src/dsky_nv_update.c src/dsky_nv_blank.c src/dsky_nv_continuation.c src/dsky_nv_return.c src/agc_multiword.c src/cm_formatter.c src/dsky_two_digit.c src/dsky_pinball.c src/dsky_decimal.c src/noun_reader.c src/fixed_data.c tests/dsky_nv_consumer_cases.c tests/dsky_nv_consumer_check.c
all: build/test-dsky-nv-consumer build/dsky-nv-consumer-route-port
build/test-dsky-nv-consumer: tests/test_dsky_nv_consumer.c $(DSKY_NV_CONSUMER_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_NV_CONSUMER_FLAGS) $^ -o $@
build/dsky-nv-consumer-route-port: tools/dsky_nv_consumer_snapshot.c $(DSKY_NV_CONSUMER_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_NV_CONSUMER_FLAGS) $^ -o $@
dsky-nv-consumer-host: build/test-dsky-nv-consumer
	build/test-dsky-nv-consumer
dsky-nv-consumer-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(DSKY_NV_CONSUMER_FLAGS) tests/test_dsky_nv_consumer.c $(DSKY_NV_CONSUMER_CORE) -o build/test-dsky-nv-consumer-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-nv-consumer-sanitize
	python3 tools/dsky_nv_consumer_sanitize.py
dsky-nv-consumer-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(filter-out -Wconversion,$(DSKY_NV_CONSUMER_FLAGS)) tools/dsky_nv_consumer_diagnostic.c $(DSKY_NV_CONSUMER_CORE) -Wl,-Map,build/dsky_nv_consumer.map -o build/dsky_nv_consumer.prg
test: dsky-nv-consumer-host
sanitize: dsky-nv-consumer-sanitize
.PHONY: dsky-nv-consumer-host dsky-nv-consumer-sanitize dsky-nv-consumer-c64
