# SPDX-License-Identifier: MIT
CM_DISPLAY_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
CM_DISPLAY_CORE = src/agc_word.c src/cm_executive.c src/cm_dispatch.c src/cm_native_image.c src/cm_basic_priority.c src/cm_init_display.c tests/cm_display_cases.c
CM_DISPLAY_TEST = tests/test_cm_display.c tests/cm_display_check.c
all: build/test-cm-display
build/test-cm-display: $(CM_DISPLAY_TEST) $(CM_DISPLAY_CORE) tests/cm_display_vectors.h
	@mkdir -p build
	$(CC) -O2 -Wconversion -pedantic $(CM_DISPLAY_FLAGS) $(CM_DISPLAY_TEST) $(CM_DISPLAY_CORE) -o $@
test-cm-display: build/test-cm-display
	build/test-cm-display
sanitize-cm-display:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CM_DISPLAY_FLAGS) $(CM_DISPLAY_TEST) $(CM_DISPLAY_CORE) -o build/test-cm-display-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-cm-display-sanitize
c64-cm-display:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(CM_DISPLAY_FLAGS) tools/cm_display_diagnostic.c tests/cm_display_check.c $(CM_DISPLAY_CORE) -Wl,-Map,build/cm_display.map -o build/cm_display.prg
differential-cm-display:
	python3 tools/cm_display_differential.py
test: test-cm-display
sanitize: sanitize-cm-display
.PHONY: test-cm-display sanitize-cm-display c64-cm-display differential-cm-display
