# SPDX-License-Identifier: MIT
RGO_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
RGO_CORE = src/agc_word.c src/p66_regodspr.c src/executive.c tests/throttle_bridge_cases.c tests/display_control_cases.c tests/regodspr_cases.c
regodspr-host:
	gcc -O2 -Wconversion -pedantic $(RGO_FLAGS) tests/test_regodspr.c tests/regodspr_check.c $(RGO_CORE) -o build/test-regodspr
	build/test-regodspr
regodspr-sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(RGO_FLAGS) tests/test_regodspr.c tests/regodspr_check.c $(RGO_CORE) -o build/test-regodspr-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-regodspr-sanitize
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(RGO_FLAGS) tools/regodspr_snapshot.c $(RGO_CORE) -o build/regodspr-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/regodspr-native-sanitize > build/regodspr-sanitize.snapshots
	cmp build/regodspr-sanitize.snapshots build/regodspr-reference.snapshots
regodspr-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(RGO_FLAGS) tools/regodspr_diagnostic.c tests/regodspr_check.c $(RGO_CORE) -Wl,-Map,build/regodspr.map -o build/regodspr.prg

test: regodspr-host
sanitize: regodspr-sanitize

.PHONY: regodspr-host regodspr-sanitize regodspr-c64 regodspr-differential
all: build/regodspr-native build/test-regodspr
build/regodspr-native: tools/regodspr_snapshot.c $(RGO_CORE) | build
	$(CC) -O2 $(RGO_FLAGS) tools/regodspr_snapshot.c $(RGO_CORE) -o $@
build/test-regodspr: tests/test_regodspr.c tests/regodspr_check.c tests/regodspr_vectors.h $(RGO_CORE) | build
	$(CC) -O2 -Wconversion -pedantic $(RGO_FLAGS) tests/test_regodspr.c tests/regodspr_check.c $(RGO_CORE) -o $@
regodspr-differential:
	python3 tools/regodspr_differential.py
