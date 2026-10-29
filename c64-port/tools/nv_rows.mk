# SPDX-License-Identifier: MIT
NVR_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
NVR_CORE = src/agc_word.c src/p66_nv_rows.c src/p66_nv_words.c src/dsky_two_digit.c src/dsky_pinball.c src/agc_multiword.c src/p66_nv_entry.c tests/throttle_bridge_cases.c tests/nv_entry_cases.c tests/nv_words_cases.c tests/nv_rows_cases.c
nv_rows-host:
	gcc -O2 -Wconversion -pedantic $(NVR_FLAGS) tests/test_nv_rows.c tests/nv_rows_check.c $(NVR_CORE) -o build/test-nv-rows
	build/test-nv-rows
nv_rows-sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(NVR_FLAGS) tests/test_nv_rows.c tests/nv_rows_check.c $(NVR_CORE) -o build/test-nv-rows-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-nv-rows-sanitize
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(NVR_FLAGS) tools/nv_rows_snapshot.c $(NVR_CORE) -o build/nv-rows-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/nv-rows-native-sanitize > build/nv-rows-sanitize.snapshots
	cmp build/nv-rows-sanitize.snapshots build/nv-rows-reference.snapshots
nv_rows-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(NVR_FLAGS) tools/nv_rows_diagnostic.c tests/nv_rows_check.c $(NVR_CORE) -Wl,-Map,build/nv_rows.map -o build/nv_rows.prg

test: nv_rows-host
sanitize: nv_rows-sanitize

.PHONY: nv_rows-host nv_rows-sanitize nv_rows-c64 nv_rows-differential
all: build/nv-rows-native build/test-nv-rows
build/nv-rows-native: tools/nv_rows_snapshot.c $(NVR_CORE) | build
	$(CC) -O2 $(NVR_FLAGS) tools/nv_rows_snapshot.c $(NVR_CORE) -o $@
build/test-nv-rows: tests/test_nv_rows.c tests/nv_rows_check.c tests/nv_rows_vectors.h $(NVR_CORE) | build
	$(CC) -O2 -Wconversion -pedantic $(NVR_FLAGS) tests/test_nv_rows.c tests/nv_rows_check.c $(NVR_CORE) -o $@
nv_rows-differential:
	python3 tools/nv_rows_differential.py
