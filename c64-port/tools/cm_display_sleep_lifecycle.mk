# SPDX-License-Identifier: MIT
CMDISPLIF_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -ffunction-sections -Wl,--gc-sections -Iinclude -Itests
CMDISPLIF_CORE = src/agc_word.c src/cm_makeplay_sleep.c src/cm_basic_wake.c src/cm_basic_sleep.c src/cm_native_image.c src/cm_dispatch.c src/cm_executive.c tests/cm_trig_cases.c tests/cm_clockjob_cases.c tests/cm_wake_cases.c tests/cm_display_sleep_lifecycle_cases.c
cm_display_sleep_lifecycle-host:
	$(CC) -O2 $(CMDISPLIF_FLAGS) tests/cm_display_sleep_lifecycle_host.c tests/cm_display_sleep_lifecycle_check.c $(CMDISPLIF_CORE) -o build/test-cm-display-sleep-lifecycle
	build/test-cm-display-sleep-lifecycle
cm_display_sleep_lifecycle-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CMDISPLIF_FLAGS) tests/cm_display_sleep_lifecycle_host.c tests/cm_display_sleep_lifecycle_check.c $(CMDISPLIF_CORE) -o build/test-cm-display-sleep-lifecycle-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-cm-display-sleep-lifecycle-sanitize
	python3 tools/cm_display_sleep_lifecycle_sanitize_stream.py
cm_display_sleep_lifecycle-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(CMDISPLIF_FLAGS) tools/cm_display_sleep_lifecycle_diagnostic.c tests/cm_display_sleep_lifecycle_check.c $(CMDISPLIF_CORE) -Wl,-Map,build/cm_display_sleep_lifecycle.map -o build/cm_display_sleep_lifecycle.prg
cm_display_sleep_lifecycle-differential:
	python3 tools/cm_display_sleep_lifecycle_differential.py
all: build/cm-display-sleep-lifecycle-native
build/cm-display-sleep-lifecycle-native: tools/cm_display_sleep_lifecycle_snapshot.c $(CMDISPLIF_CORE) | build
	$(CC) -O2 $(CMDISPLIF_FLAGS) $^ -o $@
.PHONY: cm_display_sleep_lifecycle-host cm_display_sleep_lifecycle-sanitize cm_display_sleep_lifecycle-c64 cm_display_sleep_lifecycle-differential
