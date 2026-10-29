# SPDX-License-Identifier: MIT
DSKY_DISPLAY_RELEASE_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_DISPLAY_RELEASE_CORE = src/agc_word.c src/agc_multiword.c src/executive.c src/lm_basic_wake.c src/cm_basic_wake.c src/cm_native_image.c src/cm_executive.c src/cm_dispatch.c src/dsky_display_release.c tests/p66_bias_cases.c tests/dsky_display_release_cases.c
DSKY_DISPLAY_RELEASE_TEST = tests/dsky_display_release_check.c tests/test_dsky_display_release.c
all: build/test-dsky-display-release
build/test-dsky-display-release: $(DSKY_DISPLAY_RELEASE_CORE) $(DSKY_DISPLAY_RELEASE_TEST) tests/dsky_display_release_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_DISPLAY_RELEASE_FLAGS) $(DSKY_DISPLAY_RELEASE_CORE) $(DSKY_DISPLAY_RELEASE_TEST) -o $@
test-dsky-display-release: build/test-dsky-display-release
	build/test-dsky-display-release
sanitize-dsky-display-release:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_DISPLAY_RELEASE_FLAGS) $(DSKY_DISPLAY_RELEASE_CORE) $(DSKY_DISPLAY_RELEASE_TEST) -o build/test-dsky-display-release-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-display-release-sanitize
c64-dsky-display-release:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_DISPLAY_RELEASE_FLAGS) $(DSKY_DISPLAY_RELEASE_CORE) tests/dsky_display_release_check.c tools/dsky_display_release_diagnostic.c -Wl,-Map,build/dsky_display_release.map -o build/dsky_display_release.prg
differential-dsky-display-release:
	python3 tools/dsky_display_release_differential.py
	python3 tools/freeze_dsky_display_release.py
test: test-dsky-display-release
sanitize: sanitize-dsky-display-release
.PHONY: all test sanitize test-dsky-display-release sanitize-dsky-display-release c64-dsky-display-release differential-dsky-display-release
