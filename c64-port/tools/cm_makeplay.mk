# SPDX-License-Identifier: MIT
CMMPLAY_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -ffunction-sections -Wl,--gc-sections -Iinclude -Itests
CMMPLAY_CORE = src/agc_word.c src/cm_makeplay.c tests/cm_trig_cases.c tests/cm_clockjob_cases.c tests/cm_display_control_cases.c tests/cm_makeplay_cases.c
cm_makeplay-host:
	$(CC) -O2 $(CMMPLAY_FLAGS) tests/test_cm_makeplay.c tests/cm_makeplay_check.c $(CMMPLAY_CORE) -o build/test-cm-makeplay
	build/test-cm-makeplay
cm_makeplay-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CMMPLAY_FLAGS) tests/test_cm_makeplay.c tests/cm_makeplay_check.c $(CMMPLAY_CORE) -o build/test-cm-makeplay-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-cm-makeplay-sanitize
	python3 tools/cm_makeplay_sanitize_stream.py
cm_makeplay-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(CMMPLAY_FLAGS) tools/cm_makeplay_diagnostic.c tests/cm_makeplay_check.c $(CMMPLAY_CORE) -Wl,-Map,build/cm_makeplay.map -o build/cm_makeplay.prg
cm_makeplay-differential:
	python3 tools/cm_makeplay_differential.py
all: build/cm-makeplay-native
build/cm-makeplay-native: tools/cm_makeplay_snapshot.c $(CMMPLAY_CORE) | build
	$(CC) -O2 $(CMMPLAY_FLAGS) $^ -o $@
.PHONY: cm_makeplay-host cm_makeplay-sanitize cm_makeplay-c64 cm_makeplay-differential
