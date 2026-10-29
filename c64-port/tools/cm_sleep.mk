# SPDX-License-Identifier: MIT
CMSLEEP_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -ffunction-sections -Wl,--gc-sections -Iinclude -Itests
CMSLEEP_CORE = src/agc_word.c src/cm_basic_sleep.c src/cm_native_image.c src/cm_dispatch.c src/cm_executive.c tests/cm_basic_finish_cases.c tests/cm_sleep_cases.c
cm_sleep-host:
	$(CC) -O2 $(CMSLEEP_FLAGS) tests/test_cm_sleep.c tests/cm_sleep_check.c $(CMSLEEP_CORE) -o build/test-cm-sleep
	build/test-cm-sleep
cm_sleep-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CMSLEEP_FLAGS) tests/test_cm_sleep.c tests/cm_sleep_check.c $(CMSLEEP_CORE) -o build/test-cm-sleep-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-cm-sleep-sanitize
	python3 tools/cm_sleep_sanitize_stream.py
cm_sleep-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(CMSLEEP_FLAGS) tools/cm_sleep_diagnostic.c tests/cm_sleep_check.c $(CMSLEEP_CORE) -Wl,-Map,build/cm_sleep.map -o build/cm_sleep.prg
cm_sleep-differential:
	python3 tools/cm_sleep_differential.py
all: build/cm-sleep-native
build/cm-sleep-native: tools/cm_sleep_snapshot.c $(CMSLEEP_CORE) | build
	$(CC) -O2 $(CMSLEEP_FLAGS) $^ -o $@
.PHONY: cm_sleep-host cm_sleep-sanitize cm_sleep-c64 cm_sleep-differential
