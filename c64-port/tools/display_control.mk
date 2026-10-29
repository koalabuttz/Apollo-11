# SPDX-License-Identifier: MIT
DCONT_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DCONT_CORE = src/agc_word.c src/p66_display_control.c tests/throttle_bridge_cases.c tests/display_control_cases.c
display_control-host:
	gcc -O2 -Wconversion -pedantic $(DCONT_FLAGS) tests/test_display_control.c tests/display_control_check.c $(DCONT_CORE) -o build/test-display-control
	build/test-display-control
display_control-sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DCONT_FLAGS) tests/test_display_control.c tests/display_control_check.c $(DCONT_CORE) -o build/test-display-control-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-display-control-sanitize
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DCONT_FLAGS) tools/display_control_snapshot.c $(DCONT_CORE) -o build/display-control-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/display-control-native-sanitize > build/display-control-sanitize.snapshots
	cmp build/display-control-sanitize.snapshots build/display-control-reference.snapshots
display_control-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DCONT_FLAGS) tools/display_control_diagnostic.c tests/display_control_check.c $(DCONT_CORE) -Wl,-Map,build/display_control.map -o build/display_control.prg

test: display_control-host
sanitize: display_control-sanitize

.PHONY: display_control-host display_control-sanitize display_control-c64 display_control-differential
all: build/display-control-native build/test-display-control
build/display-control-native: tools/display_control_snapshot.c $(DCONT_CORE) | build
	$(CC) -O2 $(DCONT_FLAGS) tools/display_control_snapshot.c $(DCONT_CORE) -o $@
build/test-display-control: tests/test_display_control.c tests/display_control_check.c tests/display_control_vectors.h $(DCONT_CORE) | build
	$(CC) -O2 -Wconversion -pedantic $(DCONT_FLAGS) tests/test_display_control.c tests/display_control_check.c $(DCONT_CORE) -o $@
display_control-differential:
	python3 tools/display_control_differential.py
