# SPDX-License-Identifier: MIT
NVE_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
NVE_CORE = src/agc_word.c src/p66_nv_entry.c tests/throttle_bridge_cases.c tests/nv_entry_cases.c
nv_entry-host:
	gcc -O2 -Wconversion -pedantic $(NVE_FLAGS) tests/test_nv_entry.c tests/nv_entry_check.c $(NVE_CORE) -o build/test-nv-entry
	build/test-nv-entry
nv_entry-sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(NVE_FLAGS) tests/test_nv_entry.c tests/nv_entry_check.c $(NVE_CORE) -o build/test-nv-entry-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-nv-entry-sanitize
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(NVE_FLAGS) tools/nv_entry_snapshot.c $(NVE_CORE) -o build/nv-entry-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/nv-entry-native-sanitize > build/nv-entry-sanitize.snapshots
	cmp build/nv-entry-sanitize.snapshots build/nv-entry-reference.snapshots
nv_entry-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(NVE_FLAGS) tools/nv_entry_diagnostic.c tests/nv_entry_check.c $(NVE_CORE) -Wl,-Map,build/nv_entry.map -o build/nv_entry.prg

test: nv_entry-host
sanitize: nv_entry-sanitize

.PHONY: nv_entry-host nv_entry-sanitize nv_entry-c64 nv_entry-differential
all: build/nv-entry-native build/test-nv-entry
build/nv-entry-native: tools/nv_entry_snapshot.c $(NVE_CORE) | build
	$(CC) -O2 $(NVE_FLAGS) tools/nv_entry_snapshot.c $(NVE_CORE) -o $@
build/test-nv-entry: tests/test_nv_entry.c tests/nv_entry_check.c tests/nv_entry_vectors.h $(NVE_CORE) | build
	$(CC) -O2 -Wconversion -pedantic $(NVE_FLAGS) tests/test_nv_entry.c tests/nv_entry_check.c $(NVE_CORE) -o $@
nv_entry-differential:
	python3 tools/nv_entry_differential.py
