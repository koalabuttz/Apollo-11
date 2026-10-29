# SPDX-License-Identifier: MIT
DSKY_OCTAL_DISPLAY_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_OCTAL_DISPLAY_CORE = src/agc_word.c src/dsky_pinball.c src/dsky_octal_display.c tests/p66_bias_cases.c tests/dsky_octal_display_cases.c
DSKY_OCTAL_DISPLAY_TEST = tests/dsky_octal_display_check.c tests/test_dsky_octal_display.c
all: build/test-dsky-octal-display
build/test-dsky-octal-display: $(DSKY_OCTAL_DISPLAY_CORE) $(DSKY_OCTAL_DISPLAY_TEST) tests/dsky_octal_display_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_OCTAL_DISPLAY_FLAGS) $(DSKY_OCTAL_DISPLAY_CORE) $(DSKY_OCTAL_DISPLAY_TEST) -o $@
test-dsky-octal-display: build/test-dsky-octal-display
	build/test-dsky-octal-display
sanitize-dsky-octal-display:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_OCTAL_DISPLAY_FLAGS) $(DSKY_OCTAL_DISPLAY_CORE) $(DSKY_OCTAL_DISPLAY_TEST) -o build/test-dsky-octal-display-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-octal-display-sanitize
c64-dsky-octal-display:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_OCTAL_DISPLAY_FLAGS) $(DSKY_OCTAL_DISPLAY_CORE) tests/dsky_octal_display_check.c tools/dsky_octal_display_diagnostic.c -Wl,-Map,build/dsky_octal_display.map -o build/dsky_octal_display.prg
differential-dsky-octal-display:
	python3 tools/dsky_octal_display_differential.py
	python3 tools/freeze_dsky_octal_display.py
test: test-dsky-octal-display
sanitize: sanitize-dsky-octal-display
.PHONY: all test sanitize test-dsky-octal-display sanitize-dsky-octal-display c64-dsky-octal-display differential-dsky-octal-display
