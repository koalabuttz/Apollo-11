# SPDX-License-Identifier: MIT
DSKY_BLANK_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_BLANK_CORE = src/agc_word.c src/agc_multiword.c src/dsky_pinball.c src/dsky_blank.c tests/p66_bias_cases.c tests/dsky_blank_cases.c
DSKY_BLANK_TEST = tests/dsky_blank_check.c tests/test_dsky_blank.c
all: build/test-dsky-blank
build/test-dsky-blank: $(DSKY_BLANK_CORE) $(DSKY_BLANK_TEST) tests/dsky_blank_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_BLANK_FLAGS) $(DSKY_BLANK_CORE) $(DSKY_BLANK_TEST) -o $@
test-dsky-blank: build/test-dsky-blank
	build/test-dsky-blank
sanitize-dsky-blank:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_BLANK_FLAGS) $(DSKY_BLANK_CORE) $(DSKY_BLANK_TEST) -o build/test-dsky-blank-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-blank-sanitize
c64-dsky-blank:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_BLANK_FLAGS) $(DSKY_BLANK_CORE) tests/dsky_blank_check.c tools/dsky_blank_diagnostic.c -Wl,-Map,build/dsky_blank.map -o build/dsky_blank.prg
differential-dsky-blank:
	python3 tools/dsky_blank_differential.py
	python3 tools/freeze_dsky_blank.py
test: test-dsky-blank
sanitize: sanitize-dsky-blank
.PHONY: all test sanitize test-dsky-blank sanitize-dsky-blank c64-dsky-blank differential-dsky-blank
