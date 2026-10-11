# SPDX-License-Identifier: MIT
DSKY_CHARIN_CLEAR_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_CHARIN_CLEAR_CORE = src/agc_word.c src/dsky_pinball.c src/dsky_charin_clear.c src/dsky_two_digit.c src/agc_multiword.c src/cm_formatter.c src/dsky_decimal.c tests/p66_bias_cases.c tests/dsky_charin_clear_cases.c
DSKY_CHARIN_CLEAR_TEST = tests/dsky_charin_clear_check.c tests/test_dsky_charin_clear.c
all: build/test-dsky-charin-clear
build/test-dsky-charin-clear: $(DSKY_CHARIN_CLEAR_CORE) $(DSKY_CHARIN_CLEAR_TEST) tests/dsky_charin_clear_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_CHARIN_CLEAR_FLAGS) $(DSKY_CHARIN_CLEAR_CORE) $(DSKY_CHARIN_CLEAR_TEST) -o $@
test-dsky-charin-clear: build/test-dsky-charin-clear
	build/test-dsky-charin-clear
sanitize-dsky-charin-clear:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_CHARIN_CLEAR_FLAGS) $(DSKY_CHARIN_CLEAR_CORE) $(DSKY_CHARIN_CLEAR_TEST) -o build/test-dsky-charin-clear-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-charin-clear-sanitize
c64-dsky-charin-clear:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_CHARIN_CLEAR_FLAGS) $(DSKY_CHARIN_CLEAR_CORE) tests/dsky_charin_clear_check.c tools/dsky_charin_clear_diagnostic.c -Wl,-Map,build/dsky_charin_clear.map -o build/dsky_charin_clear.prg
differential-dsky-charin-clear:
	python3 tools/dsky_charin_clear_differential.py
	python3 tools/freeze_dsky_charin_clear.py
test: test-dsky-charin-clear
sanitize: sanitize-dsky-charin-clear
.PHONY: all test sanitize test-dsky-charin-clear sanitize-dsky-charin-clear c64-dsky-charin-clear differential-dsky-charin-clear
