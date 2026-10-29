# SPDX-License-Identifier: MIT
DSKY_DECIMAL_GENERAL_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
DSKY_DECIMAL_GENERAL_CORE = src/agc_word.c src/agc_multiword.c src/dsky_pinball.c src/dsky_decimal_general.c tests/dsky_decimal_general_cases.c tests/dsky_decimal_general_check.c
all: build/test-dsky-decimal-general build/dsky-decimal-general-route-port
build/test-dsky-decimal-general: tests/test_dsky_decimal_general.c $(DSKY_DECIMAL_GENERAL_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_DECIMAL_GENERAL_FLAGS) $^ -o $@
build/dsky-decimal-general-route-port: tools/dsky_decimal_general_snapshot.c $(DSKY_DECIMAL_GENERAL_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_DECIMAL_GENERAL_FLAGS) $^ -o $@
dsky-decimal-general-host: build/test-dsky-decimal-general
	build/test-dsky-decimal-general
dsky-decimal-general-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(DSKY_DECIMAL_GENERAL_FLAGS) tests/test_dsky_decimal_general.c $(DSKY_DECIMAL_GENERAL_CORE) -o build/test-dsky-decimal-general-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-decimal-general-sanitize
	python3 tools/dsky_decimal_general_sanitize.py
dsky-decimal-general-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_DECIMAL_GENERAL_FLAGS) tools/dsky_decimal_general_diagnostic.c $(DSKY_DECIMAL_GENERAL_CORE) -Wl,-Map,build/dsky_decimal_general.map -o build/dsky_decimal_general.prg
test: dsky-decimal-general-host
sanitize: dsky-decimal-general-sanitize
.PHONY: dsky-decimal-general-host dsky-decimal-general-sanitize dsky-decimal-general-c64
