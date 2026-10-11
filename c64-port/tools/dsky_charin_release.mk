# SPDX-License-Identifier: MIT
DSKY_CHARIN_RELEASE_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_CHARIN_RELEASE_CORE = src/agc_word.c src/agc_multiword.c src/executive.c src/lm_basic_wake.c src/cm_basic_wake.c src/cm_native_image.c src/cm_executive.c src/cm_dispatch.c src/dsky_charin_release.c tests/p66_bias_cases.c tests/dsky_charin_release_cases.c
DSKY_CHARIN_RELEASE_TEST = tests/dsky_charin_release_check.c tests/test_dsky_charin_release.c
all: build/test-dsky-charin-release
build/test-dsky-charin-release: $(DSKY_CHARIN_RELEASE_CORE) $(DSKY_CHARIN_RELEASE_TEST) tests/dsky_charin_release_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_CHARIN_RELEASE_FLAGS) $(DSKY_CHARIN_RELEASE_CORE) $(DSKY_CHARIN_RELEASE_TEST) -o $@
test-dsky-charin-release: build/test-dsky-charin-release
	build/test-dsky-charin-release
sanitize-dsky-charin-release:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_CHARIN_RELEASE_FLAGS) $(DSKY_CHARIN_RELEASE_CORE) $(DSKY_CHARIN_RELEASE_TEST) -o build/test-dsky-charin-release-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-charin-release-sanitize
c64-dsky-charin-release:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_CHARIN_RELEASE_FLAGS) $(DSKY_CHARIN_RELEASE_CORE) tests/dsky_charin_release_check.c tools/dsky_charin_release_diagnostic.c -Wl,-Map,build/dsky_charin_release.map -o build/dsky_charin_release.prg
differential-dsky-charin-release:
	python3 tools/dsky_charin_release_differential.py
	python3 tools/freeze_dsky_charin_release.py
test: test-dsky-charin-release
sanitize: sanitize-dsky-charin-release
.PHONY: all test sanitize test-dsky-charin-release sanitize-dsky-charin-release c64-dsky-charin-release differential-dsky-charin-release
