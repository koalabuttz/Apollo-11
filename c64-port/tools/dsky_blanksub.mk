# SPDX-License-Identifier: MIT
DSKY_BLANKSUB_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_BLANKSUB_CORE = src/agc_word.c src/agc_multiword.c src/dsky_pinball.c src/dsky_blank.c src/dsky_blanksub.c tests/p66_bias_cases.c tests/dsky_blanksub_cases.c
DSKY_BLANKSUB_TEST = tests/dsky_blanksub_check.c tests/test_dsky_blanksub.c
all: build/test-dsky-blanksub
build/test-dsky-blanksub: $(DSKY_BLANKSUB_CORE) $(DSKY_BLANKSUB_TEST) tests/dsky_blanksub_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_BLANKSUB_FLAGS) $(DSKY_BLANKSUB_CORE) $(DSKY_BLANKSUB_TEST) -o $@
test-dsky-blanksub: build/test-dsky-blanksub
	build/test-dsky-blanksub
sanitize-dsky-blanksub:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_BLANKSUB_FLAGS) $(DSKY_BLANKSUB_CORE) $(DSKY_BLANKSUB_TEST) -o build/test-dsky-blanksub-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-blanksub-sanitize
c64-dsky-blanksub:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_BLANKSUB_FLAGS) $(DSKY_BLANKSUB_CORE) tests/dsky_blanksub_check.c tools/dsky_blanksub_diagnostic.c -Wl,-Map,build/dsky_blanksub.map -o build/dsky_blanksub.prg
differential-dsky-blanksub:
	python3 tools/dsky_blanksub_differential.py
	python3 tools/freeze_dsky_blanksub.py
test: test-dsky-blanksub
sanitize: sanitize-dsky-blanksub
.PHONY: all test sanitize test-dsky-blanksub sanitize-dsky-blanksub c64-dsky-blanksub differential-dsky-blanksub
