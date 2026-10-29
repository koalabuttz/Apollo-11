# SPDX-License-Identifier: MIT
DSKY_EXTENDED_DISPATCH_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_EXTENDED_DISPATCH_CORE = src/agc_word.c src/dsky_extended_dispatch.c tests/p66_bias_cases.c tests/dsky_extended_dispatch_cases.c
DSKY_EXTENDED_DISPATCH_TEST = tests/dsky_extended_dispatch_check.c tests/test_dsky_extended_dispatch.c
all: build/test-dsky-extended-dispatch
build/test-dsky-extended-dispatch: $(DSKY_EXTENDED_DISPATCH_CORE) $(DSKY_EXTENDED_DISPATCH_TEST) tests/dsky_extended_dispatch_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_EXTENDED_DISPATCH_FLAGS) $(DSKY_EXTENDED_DISPATCH_CORE) $(DSKY_EXTENDED_DISPATCH_TEST) -o $@
test-dsky-extended-dispatch: build/test-dsky-extended-dispatch
	build/test-dsky-extended-dispatch
sanitize-dsky-extended-dispatch:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_EXTENDED_DISPATCH_FLAGS) $(DSKY_EXTENDED_DISPATCH_CORE) $(DSKY_EXTENDED_DISPATCH_TEST) -o build/test-dsky-extended-dispatch-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-extended-dispatch-sanitize
c64-dsky-extended-dispatch:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_EXTENDED_DISPATCH_FLAGS) $(DSKY_EXTENDED_DISPATCH_CORE) tests/dsky_extended_dispatch_check.c tools/dsky_extended_dispatch_diagnostic.c -Wl,-Map,build/dsky_extended_dispatch.map -o build/dsky_extended_dispatch.prg
differential-dsky-extended-dispatch:
	python3 tools/dsky_extended_dispatch_differential.py
	python3 tools/freeze_dsky_extended_dispatch.py
test: test-dsky-extended-dispatch
sanitize: sanitize-dsky-extended-dispatch
.PHONY: all test sanitize test-dsky-extended-dispatch sanitize-dsky-extended-dispatch c64-dsky-extended-dispatch differential-dsky-extended-dispatch
