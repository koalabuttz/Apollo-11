# SPDX-License-Identifier: MIT
RTF_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
RTF_CORE = src/agc_word.c src/p66_runtime_bridge.c src/p66_runtime_fetch_bridge.c tests/throttle_bridge_cases.c tests/display_control_cases.c tests/runtime_bridge_cases.c tests/runtime_fetch_cases.c
all: build/runtime-fetch-native build/test-runtime-fetch
build/runtime-fetch-native: tools/runtime_fetch_snapshot.c $(RTF_CORE) | build
	$(CC) -O2 $(RTF_FLAGS) tools/runtime_fetch_snapshot.c $(RTF_CORE) -o $@
build/test-runtime-fetch: tests/test_runtime_fetch.c tests/runtime_fetch_check.c tests/runtime_fetch_vectors.h $(RTF_CORE) | build
	$(CC) -O2 -Wconversion -pedantic $(RTF_FLAGS) tests/test_runtime_fetch.c tests/runtime_fetch_check.c $(RTF_CORE) -o $@
runtime_fetch-host: build/test-runtime-fetch
	build/test-runtime-fetch
runtime_fetch-differential:
	python3 tools/runtime_fetch_differential.py
runtime_fetch-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(RTF_FLAGS) tests/test_runtime_fetch.c tests/runtime_fetch_check.c $(RTF_CORE) -o build/test-runtime-fetch-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-runtime-fetch-sanitize
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(RTF_FLAGS) tools/runtime_fetch_snapshot.c $(RTF_CORE) -o build/runtime-fetch-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/runtime-fetch-native-sanitize > build/runtime-fetch-sanitize.snapshots
	cmp build/runtime-fetch-sanitize.snapshots build/runtime-fetch-reference.snapshots
runtime_fetch-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(RTF_FLAGS) tools/runtime_fetch_diagnostic.c tests/runtime_fetch_check.c $(RTF_CORE) -Wl,-Map,build/runtime_fetch.map -o build/runtime_fetch.prg
.PHONY: runtime_fetch-host runtime_fetch-differential runtime_fetch-sanitize runtime_fetch-c64
test: runtime_fetch-host
sanitize: runtime_fetch-sanitize
