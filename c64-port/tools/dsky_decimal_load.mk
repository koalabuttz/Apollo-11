# SPDX-License-Identifier: MIT
DSKY_DECIMAL_LOAD_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
DSKY_DECIMAL_LOAD_CORE = src/agc_word.c src/dsky_decimal_load.c tests/dsky_decimal_load_cases.c tests/dsky_decimal_load_check.c
all: build/test-dsky-decimal-load build/dsky-decimal-load-route-port
build/test-dsky-decimal-load: tests/test_dsky_decimal_load.c $(DSKY_DECIMAL_LOAD_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_DECIMAL_LOAD_FLAGS) $^ -o $@
build/dsky-decimal-load-route-port: tools/dsky_decimal_load_snapshot.c $(DSKY_DECIMAL_LOAD_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_DECIMAL_LOAD_FLAGS) $^ -o $@
dsky-decimal-load-host: build/test-dsky-decimal-load
	build/test-dsky-decimal-load
dsky-decimal-load-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(DSKY_DECIMAL_LOAD_FLAGS) tests/test_dsky_decimal_load.c $(DSKY_DECIMAL_LOAD_CORE) -o build/test-dsky-decimal-load-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-decimal-load-sanitize
	python3 tools/dsky_decimal_load_sanitize.py
dsky-decimal-load-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_DECIMAL_LOAD_FLAGS) tools/dsky_decimal_load_diagnostic.c $(DSKY_DECIMAL_LOAD_CORE) -Wl,-Map,build/dsky_decimal_load.map -o build/dsky_decimal_load.prg
test: dsky-decimal-load-host
sanitize: dsky-decimal-load-sanitize
.PHONY: dsky-decimal-load-host dsky-decimal-load-sanitize dsky-decimal-load-c64
