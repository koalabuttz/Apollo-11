# SPDX-License-Identifier: MIT
LMSLP_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
LMSLP_CORE = src/agc_word.c src/lm_basic_sleep.c src/executive.c tests/throttle_bridge_cases.c tests/lm_priority_cases.c tests/lm_sleep_cases.c
lm_sleep-host:
	gcc -O2 -Wconversion -pedantic $(LMSLP_FLAGS) tests/test_lm_sleep.c tests/lm_sleep_check.c $(LMSLP_CORE) -o build/test-lm-sleep
	build/test-lm-sleep
lm_sleep-sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(LMSLP_FLAGS) tests/test_lm_sleep.c tests/lm_sleep_check.c $(LMSLP_CORE) -o build/test-lm-sleep-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-lm-sleep-sanitize
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(LMSLP_FLAGS) tools/lm_sleep_snapshot.c $(LMSLP_CORE) -o build/lm-sleep-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/lm-sleep-native-sanitize > build/lm-sleep-sanitize.snapshots
	cmp build/lm-sleep-sanitize.snapshots build/lm-sleep-reference.snapshots
lm_sleep-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(LMSLP_FLAGS) tools/lm_sleep_diagnostic.c tests/lm_sleep_check.c $(LMSLP_CORE) -Wl,-Map,build/lm_sleep.map -o build/lm_sleep.prg

test: lm_sleep-host
sanitize: lm_sleep-sanitize

.PHONY: lm_sleep-host lm_sleep-sanitize lm_sleep-c64 lm_sleep-differential
all: build/lm-sleep-native build/test-lm-sleep
build/lm-sleep-native: tools/lm_sleep_snapshot.c $(LMSLP_CORE) | build
	$(CC) -O2 $(LMSLP_FLAGS) tools/lm_sleep_snapshot.c $(LMSLP_CORE) -o $@
build/test-lm-sleep: tests/test_lm_sleep.c tests/lm_sleep_check.c tests/lm_sleep_vectors.h $(LMSLP_CORE) | build
	$(CC) -O2 -Wconversion -pedantic $(LMSLP_FLAGS) tests/test_lm_sleep.c tests/lm_sleep_check.c $(LMSLP_CORE) -o $@
lm_sleep-differential:
	python3 tools/lm_sleep_differential.py
