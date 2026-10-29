# SPDX-License-Identifier: MIT
DSKY_DECIMAL_SELECT_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
DSKY_DECIMAL_SELECT_CORE = src/agc_word.c src/fixed_data.c src/noun_reader.c src/dsky_decimal_select.c tests/dsky_decimal_select_cases.c tests/dsky_decimal_select_check.c
all: build/test-dsky-decimal-select build/dsky-decimal-select-route-port
build/test-dsky-decimal-select: tests/test_dsky_decimal_select.c $(DSKY_DECIMAL_SELECT_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_DECIMAL_SELECT_FLAGS) $^ -o $@
build/dsky-decimal-select-route-port: tools/dsky_decimal_select_snapshot.c $(DSKY_DECIMAL_SELECT_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_DECIMAL_SELECT_FLAGS) $^ -o $@
dsky-decimal-select-host: build/test-dsky-decimal-select
	build/test-dsky-decimal-select
dsky-decimal-select-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(DSKY_DECIMAL_SELECT_FLAGS) tests/test_dsky_decimal_select.c $(DSKY_DECIMAL_SELECT_CORE) -o build/test-dsky-decimal-select-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-decimal-select-sanitize
	python3 tools/dsky_decimal_select_sanitize.py
dsky-decimal-select-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_DECIMAL_SELECT_FLAGS) tools/dsky_decimal_select_diagnostic.c $(DSKY_DECIMAL_SELECT_CORE) -Wl,-Map,build/dsky_decimal_select.map -o build/dsky_decimal_select.prg
test: dsky-decimal-select-host
sanitize: dsky-decimal-select-sanitize
.PHONY: dsky-decimal-select-host dsky-decimal-select-sanitize dsky-decimal-select-c64
