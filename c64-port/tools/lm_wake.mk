# SPDX-License-Identifier: MIT
LMWAKE_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
LMWAKE_CORE = src/agc_word.c src/lm_basic_wake.c src/executive.c tests/throttle_bridge_cases.c tests/lm_wake_cases.c
lm_wake-host:
	gcc -O2 -Wconversion -pedantic $(LMWAKE_FLAGS) tests/test_lm_wake.c tests/lm_wake_check.c $(LMWAKE_CORE) -o build/test-lm-wake
	build/test-lm-wake
lm_wake-sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(LMWAKE_FLAGS) tests/test_lm_wake.c tests/lm_wake_check.c $(LMWAKE_CORE) -o build/test-lm-wake-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-lm-wake-sanitize
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(LMWAKE_FLAGS) tools/lm_wake_snapshot.c $(LMWAKE_CORE) -o build/lm-wake-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/lm-wake-native-sanitize > build/lm-wake-sanitize.snapshots
	cmp build/lm-wake-sanitize.snapshots build/lm-wake-reference.snapshots
lm_wake-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(LMWAKE_FLAGS) tools/lm_wake_diagnostic.c tests/lm_wake_check.c $(LMWAKE_CORE) -Wl,-Map,build/lm_wake.map -o build/lm_wake.prg

test: lm_wake-host
sanitize: lm_wake-sanitize

.PHONY: lm_wake-host lm_wake-sanitize lm_wake-c64 lm_wake-differential
all: build/lm-wake-native build/test-lm-wake
build/lm-wake-native: tools/lm_wake_snapshot.c $(LMWAKE_CORE) | build
	$(CC) -O2 $(LMWAKE_FLAGS) tools/lm_wake_snapshot.c $(LMWAKE_CORE) -o $@
build/test-lm-wake: tests/test_lm_wake.c tests/lm_wake_check.c tests/lm_wake_vectors.h $(LMWAKE_CORE) | build
	$(CC) -O2 -Wconversion -pedantic $(LMWAKE_FLAGS) tests/test_lm_wake.c tests/lm_wake_check.c $(LMWAKE_CORE) -o $@
lm_wake-differential:
	python3 tools/lm_wake_differential.py
