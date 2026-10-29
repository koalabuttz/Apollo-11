# SPDX-License-Identifier: MIT
RRGO_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
RRGO_CORE = src/agc_word.c src/p66_runtime_regodspr.c src/executive.c tests/throttle_bridge_cases.c tests/display_control_cases.c tests/regodspr_cases.c tests/runtime_regodspr_cases.c
runtime_regodspr-host:
	gcc -O2 -Wconversion -pedantic $(RRGO_FLAGS) tests/test_runtime_regodspr.c tests/runtime_regodspr_check.c $(RRGO_CORE) -o build/test-runtime_regodspr
	build/test-runtime_regodspr
runtime_regodspr-sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(RRGO_FLAGS) tests/test_runtime_regodspr.c tests/runtime_regodspr_check.c $(RRGO_CORE) -o build/test-runtime_regodspr-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-runtime_regodspr-sanitize
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(RRGO_FLAGS) tools/runtime_regodspr_snapshot.c $(RRGO_CORE) -o build/runtime_regodspr-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/runtime_regodspr-native-sanitize > build/runtime_regodspr-sanitize.snapshots
	cmp build/runtime_regodspr-sanitize.snapshots build/runtime_regodspr-reference.snapshots
runtime_regodspr-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(RRGO_FLAGS) tools/runtime_regodspr_diagnostic.c tests/runtime_regodspr_check.c $(RRGO_CORE) -Wl,-Map,build/runtime_regodspr.map -o build/runtime_regodspr.prg

test: runtime_regodspr-host
sanitize: runtime_regodspr-sanitize

.PHONY: runtime_regodspr-host runtime_regodspr-sanitize runtime_regodspr-c64 runtime_regodspr-differential
all: build/runtime_regodspr-native build/test-runtime_regodspr
build/runtime_regodspr-native: tools/runtime_regodspr_snapshot.c $(RRGO_CORE) | build
	$(CC) -O2 $(RRGO_FLAGS) tools/runtime_regodspr_snapshot.c $(RRGO_CORE) -o $@
build/test-runtime_regodspr: tests/test_runtime_regodspr.c tests/runtime_regodspr_check.c tests/runtime_regodspr_vectors.h $(RRGO_CORE) | build
	$(CC) -O2 -Wconversion -pedantic $(RRGO_FLAGS) tests/test_runtime_regodspr.c tests/runtime_regodspr_check.c $(RRGO_CORE) -o $@
runtime_regodspr-differential:
	python3 tools/runtime_regodspr_differential.py
