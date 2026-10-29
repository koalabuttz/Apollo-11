# SPDX-License-Identifier: MIT
INTEGRATED_DSKY_DECIMAL_SELECT_CORE = src/agc_word.c src/fixed_data.c src/noun_reader.c src/dsky_decimal_select.c tests/dsky_decimal_select_cases.c tests/dsky_decimal_select_check.c
INTEGRATED_DSKY_DECIMAL_SELECT_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
all: build/test-dsky-decimal-select-integrated build/dsky-decimal-select-integrated-stream
build/test-dsky-decimal-select-integrated: tests/test_dsky_decimal_select.c $(INTEGRATED_DSKY_DECIMAL_SELECT_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_DECIMAL_SELECT_FLAGS) $^ -o $@
build/dsky-decimal-select-integrated-stream: tools/dsky_decimal_select_snapshot.c $(INTEGRATED_DSKY_DECIMAL_SELECT_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_DECIMAL_SELECT_FLAGS) $^ -o $@
test: test-dsky-decimal-select-integrated
test-dsky-decimal-select-integrated: build/test-dsky-decimal-select-integrated build/dsky-decimal-select-integrated-stream
	./build/test-dsky-decimal-select-integrated
	python3 tools/qualified_stream_host.py --archive evidence/dsky-decimal-select-reference.snapshots.xz --sha256 9af2dde47f9ac87c407929c139423ec37cbd92da71f5a1b2974d453d818322f3 --binary build/dsky-decimal-select-integrated-stream
sanitize: sanitize-dsky-decimal-select-integrated
sanitize-dsky-decimal-select-integrated: | build
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_DECIMAL_SELECT_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_dsky_decimal_select.c $(INTEGRATED_DSKY_DECIMAL_SELECT_CORE) -o build/test-dsky-decimal-select-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-decimal-select-integrated-san
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_DECIMAL_SELECT_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/dsky_decimal_select_snapshot.c $(INTEGRATED_DSKY_DECIMAL_SELECT_CORE) -o build/dsky-decimal-select-integrated-stream-san
	python3 tools/qualified_stream_host.py --archive evidence/dsky-decimal-select-reference.snapshots.xz --sha256 9af2dde47f9ac87c407929c139423ec37cbd92da71f5a1b2974d453d818322f3 --binary build/dsky-decimal-select-integrated-stream-san
.PHONY: test-dsky-decimal-select-integrated sanitize-dsky-decimal-select-integrated
