# SPDX-License-Identifier: MIT
DSKY_MIXED_NOUN_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_MIXED_NOUN_CORE = src/agc_word.c src/dsky_mixed_noun.c tests/p66_bias_cases.c tests/dsky_mixed_noun_cases.c
DSKY_MIXED_NOUN_TEST = tests/dsky_mixed_noun_check.c tests/test_dsky_mixed_noun.c
all: build/test-dsky-mixed-noun
build/test-dsky-mixed-noun: $(DSKY_MIXED_NOUN_CORE) $(DSKY_MIXED_NOUN_TEST) tests/dsky_mixed_noun_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_MIXED_NOUN_FLAGS) $(DSKY_MIXED_NOUN_CORE) $(DSKY_MIXED_NOUN_TEST) -o $@
test-dsky-mixed-noun: build/test-dsky-mixed-noun
	build/test-dsky-mixed-noun
sanitize-dsky-mixed-noun:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_MIXED_NOUN_FLAGS) $(DSKY_MIXED_NOUN_CORE) $(DSKY_MIXED_NOUN_TEST) -o build/test-dsky-mixed-noun-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-mixed-noun-sanitize
c64-dsky-mixed-noun:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_MIXED_NOUN_FLAGS) $(DSKY_MIXED_NOUN_CORE) tests/dsky_mixed_noun_check.c tools/dsky_mixed_noun_diagnostic.c -Wl,-Map,build/dsky_mixed_noun.map -o build/dsky_mixed_noun.prg
differential-dsky-mixed-noun:
	python3 tools/dsky_mixed_noun_differential.py
	python3 tools/freeze_dsky_mixed_noun.py
test: test-dsky-mixed-noun
sanitize: sanitize-dsky-mixed-noun
.PHONY: all test sanitize test-dsky-mixed-noun sanitize-dsky-mixed-noun c64-dsky-mixed-noun differential-dsky-mixed-noun
