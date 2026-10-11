# SPDX-License-Identifier: MIT
DSKY_READ_PAIR_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
DSKY_READ_PAIR_CORE = src/agc_word.c src/dsky_read_pair.c tests/dsky_read_pair_cases.c tests/dsky_read_pair_check.c
all: build/test-dsky-read-pair build/dsky-read-pair-route-port
build/test-dsky-read-pair: tests/test_dsky_read_pair.c $(DSKY_READ_PAIR_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_READ_PAIR_FLAGS) $^ -o $@
build/dsky-read-pair-route-port: tools/dsky_read_pair_snapshot.c $(DSKY_READ_PAIR_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_READ_PAIR_FLAGS) $^ -o $@
dsky-read-pair-host: build/test-dsky-read-pair
	build/test-dsky-read-pair
dsky-read-pair-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(DSKY_READ_PAIR_FLAGS) tests/test_dsky_read_pair.c $(DSKY_READ_PAIR_CORE) -o build/test-dsky-read-pair-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-read-pair-sanitize
	python3 tools/dsky_read_pair_sanitize.py
dsky-read-pair-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_READ_PAIR_FLAGS) tools/dsky_read_pair_diagnostic.c $(DSKY_READ_PAIR_CORE) -Wl,-Map,build/dsky_read_pair.map -o build/dsky_read_pair.prg
test: dsky-read-pair-host
sanitize: dsky-read-pair-sanitize
.PHONY: dsky-read-pair-host dsky-read-pair-sanitize dsky-read-pair-c64
