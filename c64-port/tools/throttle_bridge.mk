# SPDX-License-Identifier: MIT
TB_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
TB_CORE = src/agc_word.c src/p66_throttle_bridge.c tests/throttle_bridge_cases.c
throttle_bridge-host:
	gcc -O2 -Wconversion -pedantic $(TB_FLAGS) tests/test_throttle_bridge.c tests/throttle_bridge_check.c $(TB_CORE) -o build/test-throttle-bridge
	build/test-throttle-bridge
throttle_bridge-sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(TB_FLAGS) tests/test_throttle_bridge.c tests/throttle_bridge_check.c $(TB_CORE) -o build/test-throttle-bridge-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-throttle-bridge-sanitize
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(TB_FLAGS) tools/throttle_bridge_snapshot.c $(TB_CORE) -o build/throttle-bridge-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/throttle-bridge-native-sanitize > build/throttle-bridge-sanitize.snapshots
	cmp build/throttle-bridge-sanitize.snapshots build/throttle-bridge-reference.snapshots
throttle_bridge-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(TB_FLAGS) tools/throttle_bridge_diagnostic.c tests/throttle_bridge_check.c $(TB_CORE) -Wl,-Map,build/throttle_bridge.map -o build/throttle_bridge.prg

test: throttle_bridge-host
sanitize: throttle_bridge-sanitize
