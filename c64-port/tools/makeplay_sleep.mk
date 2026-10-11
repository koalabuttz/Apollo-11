# SPDX-License-Identifier: MIT
MPSLP_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
MPSLP_CORE = src/agc_word.c src/p66_makeplay.c src/p66_makeplay_sleep.c tests/throttle_bridge_cases.c tests/makeplay_cases.c tests/makeplay_sleep_cases.c
all: build/makeplay-sleep-native build/test-makeplay-sleep
build/makeplay-sleep-native: tools/makeplay_sleep_snapshot.c $(MPSLP_CORE) | build
	$(CC) -O2 $(MPSLP_FLAGS) tools/makeplay_sleep_snapshot.c $(MPSLP_CORE) -o $@
build/test-makeplay-sleep: tests/test_makeplay_sleep.c tests/makeplay_sleep_check.c tests/makeplay_sleep_vectors.h $(MPSLP_CORE) | build
	$(CC) -O2 -Wconversion -pedantic $(MPSLP_FLAGS) tests/test_makeplay_sleep.c tests/makeplay_sleep_check.c $(MPSLP_CORE) -o $@
makeplay_sleep-host: build/test-makeplay-sleep
	build/test-makeplay-sleep
makeplay_sleep-differential:
	python3 tools/makeplay_sleep_differential.py
makeplay_sleep-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(MPSLP_FLAGS) tests/test_makeplay_sleep.c tests/makeplay_sleep_check.c $(MPSLP_CORE) -o build/test-makeplay-sleep-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-makeplay-sleep-sanitize
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(MPSLP_FLAGS) tools/makeplay_sleep_snapshot.c $(MPSLP_CORE) -o build/makeplay-sleep-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/makeplay-sleep-native-sanitize > build/makeplay-sleep-sanitize.snapshots
	cmp build/makeplay-sleep-sanitize.snapshots build/makeplay-sleep-reference.snapshots
makeplay_sleep-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(MPSLP_FLAGS) tools/makeplay_sleep_diagnostic.c tests/makeplay_sleep_check.c $(MPSLP_CORE) -Wl,-Map,build/makeplay_sleep.map -o build/makeplay_sleep.prg
.PHONY: makeplay_sleep-host makeplay_sleep-differential makeplay_sleep-sanitize makeplay_sleep-c64
test: makeplay_sleep-host
sanitize: makeplay_sleep-sanitize
