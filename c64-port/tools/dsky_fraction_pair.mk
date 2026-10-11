# SPDX-License-Identifier: MIT
DSKY_FRACTION_PAIR_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
DSKY_FRACTION_PAIR_CORE = src/agc_word.c src/agc_multiword.c src/dsky_pinball.c src/dsky_fraction_pair.c tests/dsky_fraction_pair_cases.c tests/dsky_fraction_pair_check.c
all: build/test-dsky-fraction-pair build/dsky-fraction-pair-route-port
build/test-dsky-fraction-pair: tests/test_dsky_fraction_pair.c $(DSKY_FRACTION_PAIR_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_FRACTION_PAIR_FLAGS) $^ -o $@
build/dsky-fraction-pair-route-port: tools/dsky_fraction_pair_snapshot.c $(DSKY_FRACTION_PAIR_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_FRACTION_PAIR_FLAGS) $^ -o $@
dsky-fraction-pair-host: build/test-dsky-fraction-pair
	build/test-dsky-fraction-pair
dsky-fraction-pair-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(DSKY_FRACTION_PAIR_FLAGS) tests/test_dsky_fraction_pair.c $(DSKY_FRACTION_PAIR_CORE) -o build/test-dsky-fraction-pair-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-fraction-pair-sanitize
	python3 tools/dsky_fraction_pair_sanitize.py
dsky-fraction-pair-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_FRACTION_PAIR_FLAGS) tools/dsky_fraction_pair_diagnostic.c $(DSKY_FRACTION_PAIR_CORE) -Wl,-Map,build/dsky_fraction_pair.map -o build/dsky_fraction_pair.prg
test: dsky-fraction-pair-host
sanitize: dsky-fraction-pair-sanitize
.PHONY: dsky-fraction-pair-host dsky-fraction-pair-sanitize dsky-fraction-pair-c64
