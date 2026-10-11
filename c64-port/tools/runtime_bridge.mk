# SPDX-License-Identifier: MIT
RTB_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
RTB_CORE = src/agc_word.c src/p66_runtime_bridge.c tests/throttle_bridge_cases.c tests/display_control_cases.c tests/runtime_bridge_cases.c
all: build/runtime-bridge-native build/test-runtime-bridge
build/runtime-bridge-native: tools/runtime_bridge_snapshot.c $(RTB_CORE) | build
	$(CC) -O2 $(RTB_FLAGS) tools/runtime_bridge_snapshot.c $(RTB_CORE) -o $@
build/test-runtime-bridge: tests/test_runtime_bridge.c tests/runtime_bridge_check.c tests/runtime_bridge_vectors.h $(RTB_CORE) | build
	$(CC) -O2 -Wconversion -pedantic $(RTB_FLAGS) tests/test_runtime_bridge.c tests/runtime_bridge_check.c $(RTB_CORE) -o $@
runtime_bridge-host: build/test-runtime-bridge
	build/test-runtime-bridge
runtime_bridge-differential:
	python3 tools/runtime_bridge_differential.py
runtime_bridge-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(RTB_FLAGS) tests/test_runtime_bridge.c tests/runtime_bridge_check.c $(RTB_CORE) -o build/test-runtime-bridge-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-runtime-bridge-sanitize
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(RTB_FLAGS) tools/runtime_bridge_snapshot.c $(RTB_CORE) -o build/runtime-bridge-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/runtime-bridge-native-sanitize > build/runtime-bridge-sanitize.snapshots
	cmp build/runtime-bridge-sanitize.snapshots build/runtime-bridge-reference.snapshots
runtime_bridge-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(RTB_FLAGS) tools/runtime_bridge_diagnostic.c tests/runtime_bridge_check.c $(RTB_CORE) -Wl,-Map,build/runtime_bridge.map -o build/runtime_bridge.prg
.PHONY: runtime_bridge-host runtime_bridge-differential runtime_bridge-sanitize runtime_bridge-c64
test: runtime_bridge-host
sanitize: runtime_bridge-sanitize
