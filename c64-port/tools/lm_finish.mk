# SPDX-License-Identifier: MIT
LMFINISH_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
LMFINISH_CORE = src/agc_word.c src/lm_basic_finish.c src/executive.c tests/throttle_bridge_cases.c tests/lm_priority_cases.c tests/lm_sleep_cases.c tests/lm_finish_cases.c
lm_finish-host:
	gcc -O2 -Wconversion -pedantic $(LMFINISH_FLAGS) tests/test_lm_finish.c tests/lm_finish_check.c $(LMFINISH_CORE) -o build/test-lm-finish
	build/test-lm-finish
lm_finish-sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(LMFINISH_FLAGS) tests/test_lm_finish.c tests/lm_finish_check.c $(LMFINISH_CORE) -o build/test-lm-finish-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-lm-finish-sanitize
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(LMFINISH_FLAGS) tools/lm_finish_snapshot.c $(LMFINISH_CORE) -o build/lm-finish-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/lm-finish-native-sanitize > build/lm-finish-sanitize.snapshots
	cmp build/lm-finish-sanitize.snapshots build/lm-finish-reference.snapshots
lm_finish-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(LMFINISH_FLAGS) tools/lm_finish_diagnostic.c tests/lm_finish_check.c $(LMFINISH_CORE) -Wl,-Map,build/lm_finish.map -o build/lm_finish.prg

test: lm_finish-host
sanitize: lm_finish-sanitize

.PHONY: lm_finish-host lm_finish-sanitize lm_finish-c64 lm_finish-differential
all: build/lm-finish-native build/test-lm-finish
build/lm-finish-native: tools/lm_finish_snapshot.c $(LMFINISH_CORE) | build
	$(CC) -O2 $(LMFINISH_FLAGS) tools/lm_finish_snapshot.c $(LMFINISH_CORE) -o $@
build/test-lm-finish: tests/test_lm_finish.c tests/lm_finish_check.c tests/lm_finish_vectors.h $(LMFINISH_CORE) | build
	$(CC) -O2 -Wconversion -pedantic $(LMFINISH_FLAGS) tests/test_lm_finish.c tests/lm_finish_check.c $(LMFINISH_CORE) -o $@
lm_finish-differential:
	python3 tools/lm_finish_differential.py
