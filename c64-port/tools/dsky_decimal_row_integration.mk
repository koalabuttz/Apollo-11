# SPDX-License-Identifier: MIT
INTEGRATED_DSKY_DECIMAL_ROW_CORE = src/agc_word.c src/dsky_decimal_row.c tests/dsky_decimal_row_cases.c tests/dsky_decimal_row_check.c
INTEGRATED_DSKY_DECIMAL_ROW_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
all: build/test-dsky-decimal-row-integrated build/dsky-decimal-row-integrated-stream
build/test-dsky-decimal-row-integrated: tests/test_dsky_decimal_row.c $(INTEGRATED_DSKY_DECIMAL_ROW_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_DECIMAL_ROW_FLAGS) $^ -o $@
build/dsky-decimal-row-integrated-stream: tools/dsky_decimal_row_snapshot.c $(INTEGRATED_DSKY_DECIMAL_ROW_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_DECIMAL_ROW_FLAGS) $^ -o $@
test: test-dsky-decimal-row-integrated
test-dsky-decimal-row-integrated: build/test-dsky-decimal-row-integrated build/dsky-decimal-row-integrated-stream
	./build/test-dsky-decimal-row-integrated
	python3 tools/qualified_stream_host.py --archive evidence/dsky-decimal-row-reference.snapshots.xz --sha256 031302d804284b6d9700a31c30505a6383c13b0b5d0dfa383016c764d7f176dd --binary build/dsky-decimal-row-integrated-stream
sanitize: sanitize-dsky-decimal-row-integrated
sanitize-dsky-decimal-row-integrated: | build
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_DECIMAL_ROW_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_dsky_decimal_row.c $(INTEGRATED_DSKY_DECIMAL_ROW_CORE) -o build/test-dsky-decimal-row-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-decimal-row-integrated-san
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_DECIMAL_ROW_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/dsky_decimal_row_snapshot.c $(INTEGRATED_DSKY_DECIMAL_ROW_CORE) -o build/dsky-decimal-row-integrated-stream-san
	python3 tools/qualified_stream_host.py --archive evidence/dsky-decimal-row-reference.snapshots.xz --sha256 031302d804284b6d9700a31c30505a6383c13b0b5d0dfa383016c764d7f176dd --binary build/dsky-decimal-row-integrated-stream-san
.PHONY: test-dsky-decimal-row-integrated sanitize-dsky-decimal-row-integrated
