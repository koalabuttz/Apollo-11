# SPDX-License-Identifier: MIT
MPLAY_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
MPLAY_CORE = src/agc_word.c src/p66_makeplay.c tests/throttle_bridge_cases.c tests/makeplay_cases.c
all: build/makeplay-native build/test-makeplay
build/makeplay-native: tools/makeplay_snapshot.c $(MPLAY_CORE) | build
	$(CC) -O2 $(MPLAY_FLAGS) tools/makeplay_snapshot.c $(MPLAY_CORE) -o $@
build/test-makeplay: tests/test_makeplay.c tests/makeplay_check.c tests/makeplay_vectors.h $(MPLAY_CORE) | build
	$(CC) -O2 -Wconversion -pedantic $(MPLAY_FLAGS) tests/test_makeplay.c tests/makeplay_check.c $(MPLAY_CORE) -o $@
makeplay-host: build/test-makeplay
	build/test-makeplay
makeplay-differential:
	python3 tools/makeplay_differential.py
makeplay-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(MPLAY_FLAGS) tests/test_makeplay.c tests/makeplay_check.c $(MPLAY_CORE) -o build/test-makeplay-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-makeplay-sanitize
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(MPLAY_FLAGS) tools/makeplay_snapshot.c $(MPLAY_CORE) -o build/makeplay-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/makeplay-native-sanitize > build/makeplay-sanitize.snapshots
	cmp build/makeplay-sanitize.snapshots build/makeplay-reference.snapshots
makeplay-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(MPLAY_FLAGS) tools/makeplay_diagnostic.c tests/makeplay_check.c $(MPLAY_CORE) -Wl,-Map,build/makeplay.map -o build/makeplay.prg
.PHONY: makeplay-host makeplay-differential makeplay-sanitize makeplay-c64
test: makeplay-host
sanitize: makeplay-sanitize
