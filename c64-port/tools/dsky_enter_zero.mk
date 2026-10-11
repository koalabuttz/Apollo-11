# SPDX-License-Identifier: MIT
DSKY_ENTER_ZERO_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_ENTER_ZERO_CORE = src/agc_word.c src/fixed_data.c src/noun_reader.c src/dsky_enter_zero.c tests/p66_bias_cases.c tests/dsky_enter_zero_cases.c
DSKY_ENTER_ZERO_TEST = tests/dsky_enter_zero_check.c tests/test_dsky_enter_zero.c
all: build/test-dsky-enter-zero
build/test-dsky-enter-zero: $(DSKY_ENTER_ZERO_CORE) $(DSKY_ENTER_ZERO_TEST) tests/dsky_enter_zero_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_ENTER_ZERO_FLAGS) $(DSKY_ENTER_ZERO_CORE) $(DSKY_ENTER_ZERO_TEST) -o $@
test-dsky-enter-zero: build/test-dsky-enter-zero
	build/test-dsky-enter-zero
sanitize-dsky-enter-zero:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_ENTER_ZERO_FLAGS) $(DSKY_ENTER_ZERO_CORE) $(DSKY_ENTER_ZERO_TEST) -o build/test-dsky-enter-zero-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-enter-zero-sanitize
c64-dsky-enter-zero:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_ENTER_ZERO_FLAGS) $(DSKY_ENTER_ZERO_CORE) tests/dsky_enter_zero_check.c tools/dsky_enter_zero_diagnostic.c -Wl,-Map,build/dsky_enter_zero.map -o build/dsky_enter_zero.prg
differential-dsky-enter-zero:
	python3 tools/dsky_enter_zero_differential.py
	python3 tools/freeze_dsky_enter_zero.py
test: test-dsky-enter-zero
sanitize: sanitize-dsky-enter-zero
.PHONY: all test sanitize test-dsky-enter-zero sanitize-dsky-enter-zero c64-dsky-enter-zero differential-dsky-enter-zero
