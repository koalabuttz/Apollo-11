# SPDX-License-Identifier: MIT
DSKY_DECIMAL_ROW_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
DSKY_DECIMAL_ROW_CORE = src/agc_word.c src/dsky_decimal_row.c tests/dsky_decimal_row_cases.c tests/dsky_decimal_row_check.c
all: build/test-dsky-decimal-row build/dsky-decimal-row-route-port
build/test-dsky-decimal-row: tests/test_dsky_decimal_row.c $(DSKY_DECIMAL_ROW_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_DECIMAL_ROW_FLAGS) $^ -o $@
build/dsky-decimal-row-route-port: tools/dsky_decimal_row_snapshot.c $(DSKY_DECIMAL_ROW_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_DECIMAL_ROW_FLAGS) $^ -o $@
dsky-decimal-row-host: build/test-dsky-decimal-row
	build/test-dsky-decimal-row
dsky-decimal-row-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(DSKY_DECIMAL_ROW_FLAGS) tests/test_dsky_decimal_row.c $(DSKY_DECIMAL_ROW_CORE) -o build/test-dsky-decimal-row-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-decimal-row-sanitize
	python3 tools/dsky_decimal_row_sanitize.py
dsky-decimal-row-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_DECIMAL_ROW_FLAGS) tools/dsky_decimal_row_diagnostic.c $(DSKY_DECIMAL_ROW_CORE) -Wl,-Map,build/dsky_decimal_row.map -o build/dsky_decimal_row.prg
test: dsky-decimal-row-host
sanitize: dsky-decimal-row-sanitize
.PHONY: dsky-decimal-row-host dsky-decimal-row-sanitize dsky-decimal-row-c64
