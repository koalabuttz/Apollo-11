# SPDX-License-Identifier: MIT
DSKY_DECIMAL_TIME_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
DSKY_DECIMAL_TIME_CORE = src/agc_word.c src/agc_multiword.c src/fixed_data.c src/noun_reader.c src/guidance_product.c src/dsky_pinball.c src/dsky_five_blank.c src/dsky_sign_output.c src/dsky_read_pair.c src/dsky_decimal_load.c src/dsky_decimal_select.c src/dsky_decimal_raw.c src/dsky_hours_output.c src/dsky_fraction_pair.c src/dsky_minutes_output.c src/dsky_two_integer.c src/dsky_decimal_time_consumer.c tests/dsky_decimal_time_consumer_cases.c tests/dsky_decimal_time_consumer_check.c
all: build/test-dsky-decimal-time-consumer build/dsky-decimal-time-consumer-route-port
build/test-dsky-decimal-time-consumer: tests/test_dsky_decimal_time_consumer.c $(DSKY_DECIMAL_TIME_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_DECIMAL_TIME_FLAGS) $^ -o $@
build/dsky-decimal-time-consumer-route-port: tools/dsky_decimal_time_consumer_snapshot.c $(DSKY_DECIMAL_TIME_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_DECIMAL_TIME_FLAGS) $^ -o $@
test: build/test-dsky-decimal-time-consumer
	build/test-dsky-decimal-time-consumer
sanitize:
	python3 tools/dsky_decimal_time_consumer_sanitize.py
