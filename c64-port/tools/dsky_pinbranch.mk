# SPDX-License-Identifier: MIT
DSKY_PINBRANCH_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_PINBRANCH_CORE = src/agc_word.c src/dsky_pinbranch.c tests/p66_bias_cases.c tests/dsky_pinbranch_cases.c
DSKY_PINBRANCH_TEST = tests/dsky_pinbranch_check.c tests/test_dsky_pinbranch.c
all: build/test-dsky-pinbranch
build/test-dsky-pinbranch: $(DSKY_PINBRANCH_CORE) $(DSKY_PINBRANCH_TEST) tests/dsky_pinbranch_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_PINBRANCH_FLAGS) $(DSKY_PINBRANCH_CORE) $(DSKY_PINBRANCH_TEST) -o $@
test-dsky-pinbranch: build/test-dsky-pinbranch
	build/test-dsky-pinbranch
sanitize-dsky-pinbranch:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_PINBRANCH_FLAGS) $(DSKY_PINBRANCH_CORE) $(DSKY_PINBRANCH_TEST) -o build/test-dsky-pinbranch-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-pinbranch-sanitize
c64-dsky-pinbranch:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_PINBRANCH_FLAGS) $(DSKY_PINBRANCH_CORE) tests/dsky_pinbranch_check.c tools/dsky_pinbranch_diagnostic.c -Wl,-Map,build/dsky_pinbranch.map -o build/dsky_pinbranch.prg
differential-dsky-pinbranch:
	python3 tools/dsky_pinbranch_differential.py
	python3 tools/freeze_dsky_pinbranch.py
test: test-dsky-pinbranch
sanitize: sanitize-dsky-pinbranch
.PHONY: all test sanitize test-dsky-pinbranch sanitize-dsky-pinbranch c64-dsky-pinbranch differential-dsky-pinbranch
