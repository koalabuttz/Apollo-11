# SPDX-License-Identifier: MIT
DSKY_CHARIN_RESET_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_CHARIN_RESET_CORE = src/agc_word.c src/dsky_pinball.c src/dsky_charin_reset.c tests/p66_bias_cases.c tests/dsky_charin_reset_cases.c
DSKY_CHARIN_RESET_TEST = tests/dsky_charin_reset_check.c tests/test_dsky_charin_reset.c
all: build/test-dsky-charin-reset
build/test-dsky-charin-reset: $(DSKY_CHARIN_RESET_CORE) $(DSKY_CHARIN_RESET_TEST) tests/dsky_charin_reset_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_CHARIN_RESET_FLAGS) $(DSKY_CHARIN_RESET_CORE) $(DSKY_CHARIN_RESET_TEST) -o $@
test-dsky-charin-reset: build/test-dsky-charin-reset
	build/test-dsky-charin-reset
sanitize-dsky-charin-reset:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_CHARIN_RESET_FLAGS) $(DSKY_CHARIN_RESET_CORE) $(DSKY_CHARIN_RESET_TEST) -o build/test-dsky-charin-reset-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-charin-reset-sanitize
c64-dsky-charin-reset:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_CHARIN_RESET_FLAGS) $(DSKY_CHARIN_RESET_CORE) tests/dsky_charin_reset_check.c tools/dsky_charin_reset_diagnostic.c -Wl,-Map,build/dsky_charin_reset.map -o build/dsky_charin_reset.prg
differential-dsky-charin-reset:
	python3 tools/dsky_charin_reset_differential.py
	python3 tools/freeze_dsky_charin_reset.py
test: test-dsky-charin-reset
sanitize: sanitize-dsky-charin-reset
.PHONY: all test sanitize test-dsky-charin-reset sanitize-dsky-charin-reset c64-dsky-charin-reset differential-dsky-charin-reset
