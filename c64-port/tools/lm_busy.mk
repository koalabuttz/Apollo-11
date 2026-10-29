# SPDX-License-Identifier: MIT
LMBUSY_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
LMBUSY_CORE = src/agc_word.c src/lm_pinball_busy.c tests/throttle_bridge_cases.c tests/lm_busy_cases.c
lm_busy-host:
	gcc -O2 -Wconversion -pedantic $(LMBUSY_FLAGS) tests/test_lm_busy.c tests/lm_busy_check.c $(LMBUSY_CORE) -o build/test-lm-busy
	build/test-lm-busy
lm_busy-sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(LMBUSY_FLAGS) tests/test_lm_busy.c tests/lm_busy_check.c $(LMBUSY_CORE) -o build/test-lm-busy-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-lm-busy-sanitize
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(LMBUSY_FLAGS) tools/lm_busy_snapshot.c $(LMBUSY_CORE) -o build/lm-busy-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/lm-busy-native-sanitize > build/lm-busy-sanitize.snapshots
	cmp build/lm-busy-sanitize.snapshots build/lm-busy-reference.snapshots
lm_busy-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(LMBUSY_FLAGS) tools/lm_busy_diagnostic.c tests/lm_busy_check.c $(LMBUSY_CORE) -Wl,-Map,build/lm_busy.map -o build/lm_busy.prg

test: lm_busy-host
sanitize: lm_busy-sanitize

.PHONY: lm_busy-host lm_busy-sanitize lm_busy-c64 lm_busy-differential
all: build/lm-busy-native build/test-lm-busy
build/lm-busy-native: tools/lm_busy_snapshot.c $(LMBUSY_CORE) | build
	$(CC) -O2 $(LMBUSY_FLAGS) tools/lm_busy_snapshot.c $(LMBUSY_CORE) -o $@
build/test-lm-busy: tests/test_lm_busy.c tests/lm_busy_check.c tests/lm_busy_vectors.h $(LMBUSY_CORE) | build
	$(CC) -O2 -Wconversion -pedantic $(LMBUSY_FLAGS) tests/test_lm_busy.c tests/lm_busy_check.c $(LMBUSY_CORE) -o $@
lm_busy-differential:
	python3 tools/lm_busy_differential.py
