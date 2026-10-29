# SPDX-License-Identifier: MIT
DSKY_ENTER_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_ENTER_CORE = src/agc_word.c src/dsky_enter.c tests/p66_bias_cases.c tests/dsky_enter_cases.c
DSKY_ENTER_TEST = tests/dsky_enter_check.c tests/test_dsky_enter.c
all: build/test-dsky-enter
build/test-dsky-enter: $(DSKY_ENTER_CORE) $(DSKY_ENTER_TEST) tests/dsky_enter_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_ENTER_FLAGS) $(DSKY_ENTER_CORE) $(DSKY_ENTER_TEST) -o $@
test-dsky-enter: build/test-dsky-enter
	build/test-dsky-enter
sanitize-dsky-enter:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_ENTER_FLAGS) $(DSKY_ENTER_CORE) $(DSKY_ENTER_TEST) -o build/test-dsky-enter-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-enter-sanitize
c64-dsky-enter:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_ENTER_FLAGS) $(DSKY_ENTER_CORE) tests/dsky_enter_check.c tools/dsky_enter_diagnostic.c -Wl,-Map,build/dsky_enter.map -o build/dsky_enter.prg
differential-dsky-enter:
	python3 tools/dsky_enter_differential.py
	python3 tools/freeze_dsky_enter.py
test: test-dsky-enter
sanitize: sanitize-dsky-enter
.PHONY: all test sanitize test-dsky-enter sanitize-dsky-enter c64-dsky-enter differential-dsky-enter
