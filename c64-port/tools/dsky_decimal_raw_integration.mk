# SPDX-License-Identifier: MIT
INTEGRATED_DSKY_DECIMAL_RAW_CORE = src/agc_word.c src/agc_multiword.c src/dsky_pinball.c src/dsky_decimal_raw.c tests/dsky_decimal_raw_cases.c tests/dsky_decimal_general_cases.c tests/dsky_decimal_raw_check.c
INTEGRATED_DSKY_DECIMAL_RAW_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
all: build/test-dsky-decimal-raw-integrated build/dsky-decimal-raw-integrated-stream
build/test-dsky-decimal-raw-integrated: tests/test_dsky_decimal_raw.c $(INTEGRATED_DSKY_DECIMAL_RAW_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_DECIMAL_RAW_FLAGS) $^ -o $@
build/dsky-decimal-raw-integrated-stream: tools/dsky_decimal_raw_snapshot.c $(INTEGRATED_DSKY_DECIMAL_RAW_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_DECIMAL_RAW_FLAGS) $^ -o $@
test: test-dsky-decimal-raw-integrated
test-dsky-decimal-raw-integrated: build/test-dsky-decimal-raw-integrated build/dsky-decimal-raw-integrated-stream
	./build/test-dsky-decimal-raw-integrated
	python3 tools/qualified_stream_host.py --archive evidence/dsky-decimal-raw-reference.snapshots.xz --sha256 8a1fda87c15f93143ec283d9538c5d011098bbe803ae925c2dd731d3e7487c25 --binary build/dsky-decimal-raw-integrated-stream
sanitize: sanitize-dsky-decimal-raw-integrated
sanitize-dsky-decimal-raw-integrated: | build
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_DECIMAL_RAW_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_dsky_decimal_raw.c $(INTEGRATED_DSKY_DECIMAL_RAW_CORE) -o build/test-dsky-decimal-raw-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-decimal-raw-integrated-san
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_DECIMAL_RAW_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/dsky_decimal_raw_snapshot.c $(INTEGRATED_DSKY_DECIMAL_RAW_CORE) -o build/dsky-decimal-raw-integrated-stream-san
	python3 tools/qualified_stream_host.py --archive evidence/dsky-decimal-raw-reference.snapshots.xz --sha256 8a1fda87c15f93143ec283d9538c5d011098bbe803ae925c2dd731d3e7487c25 --binary build/dsky-decimal-raw-integrated-stream-san
.PHONY: test-dsky-decimal-raw-integrated sanitize-dsky-decimal-raw-integrated
