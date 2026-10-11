# SPDX-License-Identifier: MIT
CM_PINBALL_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
CM_PINBALL_CORE = src/agc_word.c src/dsky_pinball.c src/cm_pinball.c tests/cm_pinball_cases.c
CM_PINBALL_TEST = tests/test_cm_pinball.c tests/cm_pinball_check.c
all: build/test-cm-pinball
build/test-cm-pinball: $(CM_PINBALL_TEST) $(CM_PINBALL_CORE) tests/cm_pinball_vectors.h
	@mkdir -p build
	$(CC) -O2 -Wconversion -pedantic $(CM_PINBALL_FLAGS) $(CM_PINBALL_TEST) $(CM_PINBALL_CORE) -o $@
test-cm-pinball: build/test-cm-pinball
	build/test-cm-pinball
sanitize-cm-pinball:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CM_PINBALL_FLAGS) $(CM_PINBALL_TEST) $(CM_PINBALL_CORE) -o build/test-cm-pinball-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-cm-pinball-sanitize
c64-cm-pinball:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(CM_PINBALL_FLAGS) tools/cm_pinball_diagnostic.c tests/cm_pinball_check.c $(CM_PINBALL_CORE) -Wl,-Map,build/cm_pinball.map -o build/cm_pinball.prg
differential-cm-pinball:
	python3 tools/cm_pinball_differential.py
test: test-cm-pinball
sanitize: sanitize-cm-pinball
.PHONY: test-cm-pinball sanitize-cm-pinball c64-cm-pinball differential-cm-pinball
