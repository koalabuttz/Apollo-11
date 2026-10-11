# SPDX-License-Identifier: MIT
DSKY_DECIMAL_RAW_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
DSKY_DECIMAL_RAW_CORE = src/agc_word.c src/agc_multiword.c src/dsky_pinball.c src/dsky_decimal_raw.c tests/dsky_decimal_raw_cases.c tests/dsky_decimal_general_cases.c tests/dsky_decimal_raw_check.c
all: build/test-dsky-decimal-raw build/dsky-decimal-raw-route-port
dsky-decimal-raw-strict: build/dsky-decimal-raw-route-port
	python3 tools/dsky_decimal_raw_strict.py
build/test-dsky-decimal-raw: tests/test_dsky_decimal_raw.c $(DSKY_DECIMAL_RAW_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_DECIMAL_RAW_FLAGS) $^ -o $@
build/dsky-decimal-raw-route-port: tools/dsky_decimal_raw_snapshot.c $(DSKY_DECIMAL_RAW_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_DECIMAL_RAW_FLAGS) $^ -o $@
dsky-decimal-raw-host: build/test-dsky-decimal-raw
	build/test-dsky-decimal-raw
dsky-decimal-raw-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(DSKY_DECIMAL_RAW_FLAGS) tests/test_dsky_decimal_raw.c $(DSKY_DECIMAL_RAW_CORE) -o build/test-dsky-decimal-raw-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-decimal-raw-sanitize
	python3 tools/dsky_decimal_raw_sanitize.py
dsky-decimal-raw-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_DECIMAL_RAW_FLAGS) tools/dsky_decimal_raw_diagnostic.c $(DSKY_DECIMAL_RAW_CORE) -Wl,-Map,build/dsky_decimal_raw.map -o build/dsky_decimal_raw.prg
test: dsky-decimal-raw-host
sanitize: dsky-decimal-raw-sanitize
.PHONY: dsky-decimal-raw-host dsky-decimal-raw-sanitize dsky-decimal-raw-c64
