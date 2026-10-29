# SPDX-License-Identifier: MIT
CMMSLEEP_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -ffunction-sections -Wl,--gc-sections -Iinclude -Itests
CMMSLEEP_CORE = src/agc_word.c src/cm_makeplay_sleep.c tests/cm_trig_cases.c tests/cm_clockjob_cases.c tests/cm_display_control_cases.c tests/cm_makeplay_sleep_cases.c
cm_makeplay_sleep-host:
	$(CC) -O2 $(CMMSLEEP_FLAGS) tests/test_cm_makeplay_sleep.c tests/cm_makeplay_sleep_check.c $(CMMSLEEP_CORE) -o build/test-cm-makeplay-sleep
	build/test-cm-makeplay-sleep
cm_makeplay_sleep-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CMMSLEEP_FLAGS) tests/test_cm_makeplay_sleep.c tests/cm_makeplay_sleep_check.c $(CMMSLEEP_CORE) -o build/test-cm-makeplay-sleep-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-cm-makeplay-sleep-sanitize
	python3 tools/cm_makeplay_sleep_sanitize_stream.py
cm_makeplay_sleep-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(CMMSLEEP_FLAGS) tools/cm_makeplay_sleep_diagnostic.c tests/cm_makeplay_sleep_check.c $(CMMSLEEP_CORE) -Wl,-Map,build/cm_makeplay_sleep.map -o build/cm_makeplay_sleep.prg
cm_makeplay_sleep-differential:
	python3 tools/cm_makeplay_sleep_differential.py
all: build/cm-makeplay-sleep-native
build/cm-makeplay-sleep-native: tools/cm_makeplay_sleep_snapshot.c $(CMMSLEEP_CORE) | build
	$(CC) -O2 $(CMMSLEEP_FLAGS) $^ -o $@
.PHONY: cm_makeplay_sleep-host cm_makeplay_sleep-sanitize cm_makeplay_sleep-c64 cm_makeplay_sleep-differential
