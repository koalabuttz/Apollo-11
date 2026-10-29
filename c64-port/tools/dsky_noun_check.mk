# SPDX-License-Identifier: MIT
DSKY_NOUN_CHECK_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_NOUN_CHECK_CORE = src/agc_word.c src/dsky_noun_check.c tests/p66_bias_cases.c tests/dsky_noun_check_cases.c
DSKY_NOUN_CHECK_TEST = tests/dsky_noun_check_check.c tests/test_dsky_noun_check.c
all: build/test-dsky-noun-check
build/test-dsky-noun-check: $(DSKY_NOUN_CHECK_CORE) $(DSKY_NOUN_CHECK_TEST) tests/dsky_noun_check_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_NOUN_CHECK_FLAGS) $(DSKY_NOUN_CHECK_CORE) $(DSKY_NOUN_CHECK_TEST) -o $@
test-dsky-noun-check: build/test-dsky-noun-check
	build/test-dsky-noun-check
sanitize-dsky-noun-check:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_NOUN_CHECK_FLAGS) $(DSKY_NOUN_CHECK_CORE) $(DSKY_NOUN_CHECK_TEST) -o build/test-dsky-noun-check-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-noun-check-sanitize
c64-dsky-noun-check:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_NOUN_CHECK_TARGET_LM $(DSKY_NOUN_CHECK_FLAGS) $(DSKY_NOUN_CHECK_CORE) tests/dsky_noun_check_check.c tools/dsky_noun_check_diagnostic.c -Wl,-Map,build/dsky_noun_check-lm.map -o build/dsky_noun_check-lm.prg
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_NOUN_CHECK_TARGET_CM $(DSKY_NOUN_CHECK_FLAGS) $(DSKY_NOUN_CHECK_CORE) tests/dsky_noun_check_check.c tools/dsky_noun_check_diagnostic.c -Wl,-Map,build/dsky_noun_check-cm.map -o build/dsky_noun_check-cm.prg
differential-dsky-noun-check:
	python3 tools/dsky_noun_check_differential.py
	python3 tools/freeze_dsky_noun_check.py
test: test-dsky-noun-check
sanitize: sanitize-dsky-noun-check
.PHONY: all test sanitize test-dsky-noun-check sanitize-dsky-noun-check c64-dsky-noun-check differential-dsky-noun-check
