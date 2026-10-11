# SPDX-License-Identifier: MIT
INTEGRATED_DSKY_NV_CONSUMER_CORE = src/agc_word.c src/dsky_nv_consumer.c src/dsky_nv_admission.c src/dsky_nv_update.c src/dsky_nv_blank.c src/dsky_nv_continuation.c src/dsky_nv_return.c src/agc_multiword.c src/cm_formatter.c src/dsky_two_digit.c src/dsky_pinball.c src/dsky_decimal.c src/noun_reader.c src/fixed_data.c tests/dsky_nv_consumer_cases.c tests/dsky_nv_consumer_check.c
INTEGRATED_DSKY_NV_CONSUMER_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
all: build/test-dsky-nv-consumer-integrated build/dsky-nv-consumer-integrated-stream
build/test-dsky-nv-consumer-integrated: tests/test_dsky_nv_consumer.c $(INTEGRATED_DSKY_NV_CONSUMER_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_NV_CONSUMER_FLAGS) $^ -o $@
build/dsky-nv-consumer-integrated-stream: tools/dsky_nv_consumer_snapshot.c $(INTEGRATED_DSKY_NV_CONSUMER_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_NV_CONSUMER_FLAGS) $^ -o $@
test: test-dsky-nv-consumer-integrated
test-dsky-nv-consumer-integrated: build/test-dsky-nv-consumer-integrated build/dsky-nv-consumer-integrated-stream
	./build/test-dsky-nv-consumer-integrated
	python3 tools/qualified_stream_host.py --archive evidence/dsky-nv-consumer-reference.snapshots.xz --sha256 408ccb8b999dc1802b4321630029cf6852a3ce9f2a80a7e8ceac854f667d9e99 --binary build/dsky-nv-consumer-integrated-stream
sanitize: sanitize-dsky-nv-consumer-integrated
sanitize-dsky-nv-consumer-integrated: | build
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_NV_CONSUMER_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_dsky_nv_consumer.c $(INTEGRATED_DSKY_NV_CONSUMER_CORE) -o build/test-dsky-nv-consumer-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-nv-consumer-integrated-san
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_NV_CONSUMER_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/dsky_nv_consumer_snapshot.c $(INTEGRATED_DSKY_NV_CONSUMER_CORE) -o build/dsky-nv-consumer-integrated-stream-san
	python3 tools/qualified_stream_host.py --archive evidence/dsky-nv-consumer-reference.snapshots.xz --sha256 408ccb8b999dc1802b4321630029cf6852a3ce9f2a80a7e8ceac854f667d9e99 --binary build/dsky-nv-consumer-integrated-stream-san
.PHONY: test-dsky-nv-consumer-integrated sanitize-dsky-nv-consumer-integrated
