# SPDX-License-Identifier: MIT
INTEGRATED_DSKY_DECIMAL_CONSUMER_CORE = src/agc_word.c src/agc_multiword.c src/fixed_data.c src/noun_reader.c src/guidance_product.c src/dsky_scale.c src/cm_scale.c src/dsky_pinball.c src/dsky_decimal_load.c src/dsky_decimal_select.c src/dsky_scalar_output.c src/dsky_decimal_row.c src/dsky_decimal_general.c src/dsky_decimal_raw.c src/dsky_decimal_consumer.c tests/dsky_decimal_consumer_cases.c tests/dsky_decimal_consumer_check.c
INTEGRATED_DSKY_DECIMAL_CONSUMER_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
all: build/test-dsky-decimal-consumer-integrated build/dsky-decimal-consumer-integrated-stream
build/test-dsky-decimal-consumer-integrated: tests/test_dsky_decimal_consumer.c $(INTEGRATED_DSKY_DECIMAL_CONSUMER_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_DECIMAL_CONSUMER_FLAGS) $^ -o $@
build/dsky-decimal-consumer-integrated-stream: tools/dsky_decimal_consumer_snapshot.c $(INTEGRATED_DSKY_DECIMAL_CONSUMER_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_DECIMAL_CONSUMER_FLAGS) $^ -o $@
test: test-dsky-decimal-consumer-integrated
test-dsky-decimal-consumer-integrated: build/test-dsky-decimal-consumer-integrated build/dsky-decimal-consumer-integrated-stream
	./build/test-dsky-decimal-consumer-integrated
	python3 tools/qualified_stream_host.py --archive evidence/dsky-decimal-consumer-reference.snapshots.xz --sha256 5aad80120d2b942b726a080b7d95ea004e4836a49772fc485ef0ca8a0ed4a474 --binary build/dsky-decimal-consumer-integrated-stream
sanitize: sanitize-dsky-decimal-consumer-integrated
sanitize-dsky-decimal-consumer-integrated: | build
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_DECIMAL_CONSUMER_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_dsky_decimal_consumer.c $(INTEGRATED_DSKY_DECIMAL_CONSUMER_CORE) -o build/test-dsky-decimal-consumer-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-decimal-consumer-integrated-san
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_DECIMAL_CONSUMER_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/dsky_decimal_consumer_snapshot.c $(INTEGRATED_DSKY_DECIMAL_CONSUMER_CORE) -o build/dsky-decimal-consumer-integrated-stream-san
	python3 tools/qualified_stream_host.py --archive evidence/dsky-decimal-consumer-reference.snapshots.xz --sha256 5aad80120d2b942b726a080b7d95ea004e4836a49772fc485ef0ca8a0ed4a474 --binary build/dsky-decimal-consumer-integrated-stream-san
.PHONY: test-dsky-decimal-consumer-integrated sanitize-dsky-decimal-consumer-integrated
