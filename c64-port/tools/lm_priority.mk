# SPDX-License-Identifier: MIT
LMPRI_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
LMPRI_CORE = src/agc_word.c src/lm_basic_priority.c src/executive.c tests/throttle_bridge_cases.c tests/lm_priority_cases.c
lm_priority-host:
	gcc -O2 -Wconversion -pedantic $(LMPRI_FLAGS) tests/test_lm_priority.c tests/lm_priority_check.c $(LMPRI_CORE) -o build/test-lm-priority
	build/test-lm-priority
lm_priority-sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(LMPRI_FLAGS) tests/test_lm_priority.c tests/lm_priority_check.c $(LMPRI_CORE) -o build/test-lm-priority-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-lm-priority-sanitize
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(LMPRI_FLAGS) tools/lm_priority_snapshot.c $(LMPRI_CORE) -o build/lm-priority-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/lm-priority-native-sanitize > build/lm-priority-sanitize.snapshots
	cmp build/lm-priority-sanitize.snapshots build/lm-priority-reference.snapshots
lm_priority-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(LMPRI_FLAGS) tools/lm_priority_diagnostic.c tests/lm_priority_check.c $(LMPRI_CORE) -Wl,-Map,build/lm_priority.map -o build/lm_priority.prg

test: lm_priority-host
sanitize: lm_priority-sanitize

.PHONY: lm_priority-host lm_priority-sanitize lm_priority-c64 lm_priority-differential
all: build/lm-priority-native build/test-lm-priority
build/lm-priority-native: tools/lm_priority_snapshot.c $(LMPRI_CORE) | build
	$(CC) -O2 $(LMPRI_FLAGS) tools/lm_priority_snapshot.c $(LMPRI_CORE) -o $@
build/test-lm-priority: tests/test_lm_priority.c tests/lm_priority_check.c tests/lm_priority_vectors.h $(LMPRI_CORE) | build
	$(CC) -O2 -Wconversion -pedantic $(LMPRI_FLAGS) tests/test_lm_priority.c tests/lm_priority_check.c $(LMPRI_CORE) -o $@
lm_priority-differential:
	python3 tools/lm_priority_differential.py
