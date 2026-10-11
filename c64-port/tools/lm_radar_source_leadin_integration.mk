# SPDX-License-Identifier: MIT
CC ?= cc
RADAR_FLAGS=-std=c99 -Wall -Wextra -Werror -Iinclude -Itests -ffunction-sections -fdata-sections -Wl,--gc-sections -DP66_NATIVE_DISPATCH_ONLY
RADAR_SOURCES=src/agc_word.c src/agc_multiword.c src/waitlist.c src/executive.c src/p66_native.c src/lm_waitlist_image.c src/lm_admission_image.c src/lm_radar_leadin.c tests/lm_admission_image_cases.c tests/lm_radar_leadin_cases.c
test-lm-radar-source-leadin:
	$(CC) $(RADAR_FLAGS) -O2 $(RADAR_SOURCES) tests/test_lm_radar_leadin.c -o build/test-lm-radar-leadin
	build/test-lm-radar-leadin
	python3 tools/lm_radar_leadin_differential.py --retained
sanitize-lm-radar-source-leadin:
	$(CC) $(RADAR_FLAGS) -O1 -g -no-pie -fsanitize=address,undefined $(RADAR_SOURCES) tests/test_lm_radar_leadin.c -o build/test-lm-radar-leadin-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-lm-radar-leadin-sanitize
	python3 tools/lm_radar_leadin_differential.py --retained --sanitize

test: test-lm-radar-source-leadin
sanitize: sanitize-lm-radar-source-leadin
.PHONY: test-lm-radar-source-leadin sanitize-lm-radar-source-leadin
