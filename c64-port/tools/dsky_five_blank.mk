# SPDX-License-Identifier: MIT
DSKY_FIVE_BLANK_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_FIVE_BLANK_CORE = src/agc_word.c src/dsky_pinball.c src/dsky_five_blank.c tests/p66_bias_cases.c tests/dsky_five_blank_cases.c
DSKY_FIVE_BLANK_TEST = tests/dsky_five_blank_check.c tests/test_dsky_five_blank.c
all: build/test-dsky-five-blank
build/test-dsky-five-blank: $(DSKY_FIVE_BLANK_CORE) $(DSKY_FIVE_BLANK_TEST) tests/dsky_five_blank_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_FIVE_BLANK_FLAGS) $(DSKY_FIVE_BLANK_CORE) $(DSKY_FIVE_BLANK_TEST) -o $@
test-dsky-five-blank: build/test-dsky-five-blank
	build/test-dsky-five-blank
sanitize-dsky-five-blank:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_FIVE_BLANK_FLAGS) $(DSKY_FIVE_BLANK_CORE) $(DSKY_FIVE_BLANK_TEST) -o build/test-dsky-five-blank-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-five-blank-sanitize
c64-dsky-five-blank:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_FIVE_BLANK_FLAGS) $(DSKY_FIVE_BLANK_CORE) tests/dsky_five_blank_check.c tools/dsky_five_blank_diagnostic.c -Wl,-Map,build/dsky_five_blank.map -o build/dsky_five_blank.prg
differential-dsky-five-blank:
	python3 tools/dsky_five_blank_differential.py
	python3 tools/freeze_dsky_five_blank.py
test: test-dsky-five-blank
sanitize: sanitize-dsky-five-blank
.PHONY: all test sanitize test-dsky-five-blank sanitize-dsky-five-blank c64-dsky-five-blank differential-dsky-five-blank
