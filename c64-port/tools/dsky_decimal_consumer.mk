# SPDX-License-Identifier: MIT
DSKY_DECIMAL_CONSUMER_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
DSKY_DECIMAL_CONSUMER_CORE = src/agc_word.c src/agc_multiword.c src/fixed_data.c src/noun_reader.c src/guidance_product.c src/dsky_scale.c src/cm_scale.c src/dsky_pinball.c src/dsky_decimal_load.c src/dsky_decimal_select.c src/dsky_scalar_output.c src/dsky_decimal_row.c src/dsky_decimal_general.c src/dsky_decimal_raw.c src/dsky_decimal_consumer.c tests/dsky_decimal_consumer_cases.c tests/dsky_decimal_consumer_check.c
all: build/test-dsky-decimal-consumer build/dsky-decimal-consumer-route-port
build/test-dsky-decimal-consumer: tests/test_dsky_decimal_consumer.c $(DSKY_DECIMAL_CONSUMER_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_DECIMAL_CONSUMER_FLAGS) $^ -o $@
build/dsky-decimal-consumer-route-port: tools/dsky_decimal_consumer_snapshot.c $(DSKY_DECIMAL_CONSUMER_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_DECIMAL_CONSUMER_FLAGS) $^ -o $@
dsky-decimal-consumer-host: build/test-dsky-decimal-consumer
	build/test-dsky-decimal-consumer
dsky-decimal-consumer-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(DSKY_DECIMAL_CONSUMER_FLAGS) tests/test_dsky_decimal_consumer.c $(DSKY_DECIMAL_CONSUMER_CORE) -o build/test-dsky-decimal-consumer-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-decimal-consumer-sanitize
	python3 tools/dsky_decimal_consumer_sanitize.py
dsky-decimal-consumer-c64:
	python3 tools/dsky_decimal_consumer_build.py --mos-cc $(MOS_CC)
test: dsky-decimal-consumer-host
sanitize: dsky-decimal-consumer-sanitize
.PHONY: dsky-decimal-consumer-host dsky-decimal-consumer-sanitize dsky-decimal-consumer-c64
