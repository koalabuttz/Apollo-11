# SPDX-License-Identifier: MIT
CM_FORMATTER_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
CM_FORMATTER_CORE = src/agc_word.c src/agc_multiword.c src/dsky_pinball.c src/dsky_two_digit.c src/dsky_decimal.c src/cm_formatter.c tests/cm_pinball_cases.c tests/cm_formatter_cases.c
CM_FORMATTER_TEST = tests/test_cm_formatter.c tests/cm_formatter_check.c
all: build/test-cm-formatter
build/test-cm-formatter: $(CM_FORMATTER_TEST) $(CM_FORMATTER_CORE) tests/cm_formatter_vectors.h
	@mkdir -p build
	$(CC) -O2 -Wconversion -pedantic $(CM_FORMATTER_FLAGS) $(CM_FORMATTER_TEST) $(CM_FORMATTER_CORE) -o $@
test-cm-formatter: build/test-cm-formatter
	build/test-cm-formatter
sanitize-cm-formatter:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CM_FORMATTER_FLAGS) $(CM_FORMATTER_TEST) $(CM_FORMATTER_CORE) -o build/test-cm-formatter-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-cm-formatter-sanitize
c64-cm-formatter:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(CM_FORMATTER_FLAGS) tools/cm_formatter_diagnostic.c tests/cm_formatter_check.c $(CM_FORMATTER_CORE) -Wl,-Map,build/cm_formatter.map -o build/cm_formatter.prg
differential-cm-formatter:
	python3 tools/cm_formatter_differential.py
test: test-cm-formatter
sanitize: sanitize-cm-formatter
.PHONY: test-cm-formatter sanitize-cm-formatter c64-cm-formatter differential-cm-formatter
