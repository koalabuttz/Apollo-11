# SPDX-License-Identifier: MIT
INTEGRATED_DSKY_DECIMAL_TIME_CONSUMER_CORE = src/agc_word.c src/agc_multiword.c src/fixed_data.c src/noun_reader.c src/guidance_product.c src/dsky_pinball.c src/dsky_five_blank.c src/dsky_sign_output.c src/dsky_read_pair.c src/dsky_decimal_load.c src/dsky_decimal_select.c src/dsky_decimal_raw.c src/dsky_hours_output.c src/dsky_fraction_pair.c src/dsky_minutes_output.c src/dsky_two_integer.c src/dsky_decimal_time_consumer.c tests/dsky_decimal_time_consumer_cases.c tests/dsky_decimal_time_consumer_check.c
INTEGRATED_DSKY_DECIMAL_TIME_CONSUMER_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
all: build/test-dsky-decimal-time-consumer-integrated build/dsky-decimal-time-consumer-integrated-stream
build/test-dsky-decimal-time-consumer-integrated: tests/test_dsky_decimal_time_consumer.c $(INTEGRATED_DSKY_DECIMAL_TIME_CONSUMER_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_DECIMAL_TIME_CONSUMER_FLAGS) $^ -o $@
build/dsky-decimal-time-consumer-integrated-stream: tools/dsky_decimal_time_consumer_snapshot.c $(INTEGRATED_DSKY_DECIMAL_TIME_CONSUMER_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_DECIMAL_TIME_CONSUMER_FLAGS) $^ -o $@
test: test-dsky-decimal-time-consumer-integrated
test-dsky-decimal-time-consumer-integrated: build/test-dsky-decimal-time-consumer-integrated build/dsky-decimal-time-consumer-integrated-stream
	./build/test-dsky-decimal-time-consumer-integrated
	python3 tools/qualified_stream_host.py --archive evidence/dsky-decimal-time-consumer-reference.snapshots.xz --sha256 26cf16d5a756b261c2a3fc64d2fb4160d13ef52e34bdb611ce2f14f8ffe4a5b6 --binary build/dsky-decimal-time-consumer-integrated-stream
sanitize: sanitize-dsky-decimal-time-consumer-integrated
sanitize-dsky-decimal-time-consumer-integrated: | build
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_DECIMAL_TIME_CONSUMER_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_dsky_decimal_time_consumer.c $(INTEGRATED_DSKY_DECIMAL_TIME_CONSUMER_CORE) -o build/test-dsky-decimal-time-consumer-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-decimal-time-consumer-integrated-san
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_DECIMAL_TIME_CONSUMER_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/dsky_decimal_time_consumer_snapshot.c $(INTEGRATED_DSKY_DECIMAL_TIME_CONSUMER_CORE) -o build/dsky-decimal-time-consumer-integrated-stream-san
	python3 tools/qualified_stream_host.py --archive evidence/dsky-decimal-time-consumer-reference.snapshots.xz --sha256 26cf16d5a756b261c2a3fc64d2fb4160d13ef52e34bdb611ce2f14f8ffe4a5b6 --binary build/dsky-decimal-time-consumer-integrated-stream-san
.PHONY: test-dsky-decimal-time-consumer-integrated sanitize-dsky-decimal-time-consumer-integrated
