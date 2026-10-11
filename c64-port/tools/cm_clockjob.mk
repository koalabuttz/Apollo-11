# SPDX-License-Identifier: MIT
CMCLK_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
CMCLK_CORE = src/agc_word.c src/cm_clockjob.c tests/cm_trig_cases.c tests/cm_clockjob_cases.c
cm_clockjob-host:
	$(CC) -O2 $(CMCLK_FLAGS) tests/test_cm_clockjob.c tests/cm_clockjob_check.c $(CMCLK_CORE) -o build/test-cm-clockjob
	build/test-cm-clockjob
cm_clockjob-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CMCLK_FLAGS) tests/test_cm_clockjob.c tests/cm_clockjob_check.c $(CMCLK_CORE) -o build/test-cm-clockjob-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-cm-clockjob-sanitize
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CMCLK_FLAGS) tools/cm_clockjob_snapshot.c $(CMCLK_CORE) -o build/cm-clockjob-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/cm-clockjob-sanitize > build/cm-clockjob-sanitize.snapshots
	cmp build/cm-clockjob-sanitize.snapshots build/cm-clockjob-reference.snapshots
cm_clockjob-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(CMCLK_FLAGS) tools/cm_clockjob_diagnostic.c tests/cm_clockjob_check.c $(CMCLK_CORE) -Wl,-Map,build/cm_clockjob.map -o build/cm_clockjob.prg
cm_clockjob-differential:
	python3 tools/cm_clockjob_differential.py
all: build/cm-clockjob-native
build/cm-clockjob-native: tools/cm_clockjob_snapshot.c $(CMCLK_CORE) | build
	$(CC) -O2 $(CMCLK_FLAGS) $^ -o $@
.PHONY: cm_clockjob-host cm_clockjob-sanitize cm_clockjob-c64 cm_clockjob-differential
cm_clockjob-full:
	python3 tools/cm_clockjob_full_differential.py
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CMCLK_FLAGS) tools/cm_clockjob_full_snapshot.c $(CMCLK_CORE) src/cm_quictrig.c src/agc_multiword.c -o build/cm-clockjob-full-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/cm-clockjob-full-sanitize > build/cm-clockjob-full-sanitize.snapshots
	cmp build/cm-clockjob-full-sanitize.snapshots build/cm-clockjob-full-reference.snapshots
