# SPDX-License-Identifier: MIT
DSKY_CHARIN_SIGN_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_CHARIN_SIGN_CORE = src/agc_word.c src/dsky_pinball.c src/dsky_charin_sign.c tests/p66_bias_cases.c tests/dsky_charin_sign_cases.c
DSKY_CHARIN_SIGN_TEST = tests/dsky_charin_sign_check.c tests/test_dsky_charin_sign.c
all: build/test-dsky-charin-sign
build/test-dsky-charin-sign: $(DSKY_CHARIN_SIGN_CORE) $(DSKY_CHARIN_SIGN_TEST) tests/dsky_charin_sign_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_CHARIN_SIGN_FLAGS) $(DSKY_CHARIN_SIGN_CORE) $(DSKY_CHARIN_SIGN_TEST) -o $@
test-dsky-charin-sign: build/test-dsky-charin-sign
	build/test-dsky-charin-sign
sanitize-dsky-charin-sign:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_CHARIN_SIGN_FLAGS) $(DSKY_CHARIN_SIGN_CORE) $(DSKY_CHARIN_SIGN_TEST) -o build/test-dsky-charin-sign-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-charin-sign-sanitize
c64-dsky-charin-sign:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_CHARIN_SIGN_FLAGS) $(DSKY_CHARIN_SIGN_CORE) tests/dsky_charin_sign_check.c tools/dsky_charin_sign_diagnostic.c -Wl,-Map,build/dsky_charin_sign.map -o build/dsky_charin_sign.prg
differential-dsky-charin-sign:
	python3 tools/dsky_charin_sign_differential.py
	python3 tools/freeze_dsky_charin_sign.py
test: test-dsky-charin-sign
sanitize: sanitize-dsky-charin-sign
.PHONY: all test sanitize test-dsky-charin-sign sanitize-dsky-charin-sign c64-dsky-charin-sign differential-dsky-charin-sign
