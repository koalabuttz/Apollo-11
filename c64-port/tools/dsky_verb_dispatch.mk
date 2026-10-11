# SPDX-License-Identifier: MIT
DSKY_VERB_DISPATCH_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_VERB_DISPATCH_CORE = src/agc_word.c src/dsky_verb_dispatch.c tests/p66_bias_cases.c tests/dsky_verb_dispatch_cases.c
DSKY_VERB_DISPATCH_TEST = tests/dsky_verb_dispatch_check.c tests/test_dsky_verb_dispatch.c
all: build/test-dsky-verb-dispatch
build/test-dsky-verb-dispatch: $(DSKY_VERB_DISPATCH_CORE) $(DSKY_VERB_DISPATCH_TEST) tests/dsky_verb_dispatch_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_VERB_DISPATCH_FLAGS) $(DSKY_VERB_DISPATCH_CORE) $(DSKY_VERB_DISPATCH_TEST) -o $@
test-dsky-verb-dispatch: build/test-dsky-verb-dispatch
	build/test-dsky-verb-dispatch
sanitize-dsky-verb-dispatch:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_VERB_DISPATCH_FLAGS) $(DSKY_VERB_DISPATCH_CORE) $(DSKY_VERB_DISPATCH_TEST) -o build/test-dsky-verb-dispatch-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-verb-dispatch-sanitize
c64-dsky-verb-dispatch:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_VERB_DISPATCH_FLAGS) $(DSKY_VERB_DISPATCH_CORE) tests/dsky_verb_dispatch_check.c tools/dsky_verb_dispatch_diagnostic.c -Wl,-Map,build/dsky_verb_dispatch.map -o build/dsky_verb_dispatch.prg
differential-dsky-verb-dispatch:
	python3 tools/dsky_verb_dispatch_differential.py
	python3 tools/freeze_dsky_verb_dispatch.py
test: test-dsky-verb-dispatch
sanitize: sanitize-dsky-verb-dispatch
.PHONY: all test sanitize test-dsky-verb-dispatch sanitize-dsky-verb-dispatch c64-dsky-verb-dispatch differential-dsky-verb-dispatch
