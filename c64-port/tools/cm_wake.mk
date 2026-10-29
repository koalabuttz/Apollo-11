# SPDX-License-Identifier: MIT
CMWAKE_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -ffunction-sections -Wl,--gc-sections -Iinclude -Itests
CMWAKE_CORE = src/agc_word.c src/cm_basic_wake.c src/cm_native_image.c src/cm_dispatch.c src/cm_executive.c tests/cm_trig_cases.c tests/cm_clockjob_cases.c tests/cm_wake_cases.c
cm_wake-host:
	$(CC) -O2 $(CMWAKE_FLAGS) tests/test_cm_wake.c tests/cm_wake_check.c $(CMWAKE_CORE) -o build/test-cm-wake
	build/test-cm-wake
cm_wake-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CMWAKE_FLAGS) tests/test_cm_wake.c tests/cm_wake_check.c $(CMWAKE_CORE) -o build/test-cm-wake-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-cm-wake-sanitize
	python3 tools/cm_wake_sanitize_stream.py
cm_wake-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(CMWAKE_FLAGS) tools/cm_wake_diagnostic.c tests/cm_wake_check.c $(CMWAKE_CORE) -Wl,-Map,build/cm_wake.map -o build/cm_wake.prg
cm_wake-differential:
	python3 tools/cm_wake_differential.py
all: build/cm-wake-native
build/cm-wake-native: tools/cm_wake_snapshot.c $(CMWAKE_CORE) | build
	$(CC) -O2 $(CMWAKE_FLAGS) $^ -o $@
.PHONY: cm_wake-host cm_wake-sanitize cm_wake-c64 cm_wake-differential
