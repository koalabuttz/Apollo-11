# SPDX-License-Identifier: MIT
CM_CHARIN_NUMBER_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
CM_CHARIN_NUMBER_CORE = src/agc_word.c src/agc_multiword.c src/dsky_pinball.c src/dsky_charin_number.c src/cm_charin_number.c tests/p66_bias_cases.c tests/dsky_nv_key_cases.c tests/dsky_charin_number_cases.c tests/cm_charin_number_cases.c
CM_CHARIN_NUMBER_TEST = tests/test_cm_charin_number.c tests/cm_charin_number_check.c
all: build/test-cm-charin-number
build/test-cm-charin-number: $(CM_CHARIN_NUMBER_CORE) $(CM_CHARIN_NUMBER_TEST) tests/cm_charin_number_vectors.h
	@mkdir -p build
	$(CC) -O2 -Wconversion -pedantic $(CM_CHARIN_NUMBER_FLAGS) $(CM_CHARIN_NUMBER_CORE) $(CM_CHARIN_NUMBER_TEST) -o $@
test-cm-charin-number: build/test-cm-charin-number
	build/test-cm-charin-number
sanitize-cm-charin-number:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CM_CHARIN_NUMBER_FLAGS) $(CM_CHARIN_NUMBER_CORE) $(CM_CHARIN_NUMBER_TEST) -o build/test-cm-charin-number-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-cm-charin-number-sanitize
c64-cm-charin-number:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(CM_CHARIN_NUMBER_FLAGS) $(CM_CHARIN_NUMBER_CORE) tests/cm_charin_number_check.c tools/cm_charin_number_diagnostic.c -Wl,-Map,build/cm_charin_number.map -o build/cm_charin_number.prg
differential-cm-charin-number:
	python3 tools/cm_charin_number_differential.py
	python3 tools/freeze_cm_charin_number.py
test: test-cm-charin-number
sanitize: sanitize-cm-charin-number
.PHONY: all test sanitize test-cm-charin-number sanitize-cm-charin-number c64-cm-charin-number differential-cm-charin-number
