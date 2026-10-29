# SPDX-License-Identifier: MIT
SHELL = /bin/bash
.SHELLFLAGS = -eu -o pipefail -c
SQUARE_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
SQUARE_CORE = src/interpreted_square.c src/agc_word.c src/agc_multiword.c tests/interpreted_square_cases.c
all: build/interpreted-square-native build/interpreted-square-reference
build/interpreted-square-native: tools/interpreted_square_snapshot.c $(SQUARE_CORE)
	gcc -O2 $(SQUARE_FLAGS) tools/interpreted_square_snapshot.c $(SQUARE_CORE) -o $@
build/interpreted-square-reference: tools/interpreted_square_reference.c tests/interpreted_square_cases.c
	gcc -O2 -std=c99 -Iinclude -Itests -Ideps/virtualagc/yaAGC tools/interpreted_square_reference.c tests/interpreted_square_cases.c tests/cm_executive_cases.c src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	build/interpreted-square-reference deps/lm-powered-fixture/MAIN.agc.bin 0 2> build/interpreted-square-lm-reference.trace | gzip -n -9 > build/interpreted-square-lm-reference.snapshots.gz
	build/interpreted-square-native 0 | gzip -n -9 > build/interpreted-square-lm-native.snapshots.gz
	cmp build/interpreted-square-lm-reference.snapshots.gz build/interpreted-square-lm-native.snapshots.gz
	build/interpreted-square-reference deps/cm-powered-fixture/MAIN.agc.bin 1 2> build/interpreted-square-cm-reference.trace | gzip -n -9 > build/interpreted-square-cm-reference.snapshots.gz
	build/interpreted-square-native 1 | gzip -n -9 > build/interpreted-square-cm-native.snapshots.gz
	cmp build/interpreted-square-cm-reference.snapshots.gz build/interpreted-square-cm-native.snapshots.gz
sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(SQUARE_FLAGS) tools/interpreted_square_snapshot.c $(SQUARE_CORE) -o build/interpreted-square-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/interpreted-square-sanitize 0 | gzip -n -9 > build/interpreted-square-lm-sanitize.snapshots.gz
	cmp build/interpreted-square-lm-reference.snapshots.gz build/interpreted-square-lm-sanitize.snapshots.gz
	ASAN_OPTIONS=detect_leaks=0 build/interpreted-square-sanitize 1 | gzip -n -9 > build/interpreted-square-cm-sanitize.snapshots.gz
	cmp build/interpreted-square-cm-reference.snapshots.gz build/interpreted-square-cm-sanitize.snapshots.gz
.PHONY: all test sanitize
check-host:
	gcc -O2 $(SQUARE_FLAGS) tests/test_interpreted_square.c tests/frozen_words.c $(SQUARE_CORE) -o build/test-interpreted-square
	build/test-interpreted-square
c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DINTERPRETED_SQUARE_CHECK_ONLY -DINTERPRETED_SQUARE_SELECTED=$(if $(SELECTOR),$(SELECTOR),0) $(SQUARE_FLAGS) tests/test_interpreted_square.c tests/frozen_words.c tools/interpreted_square_diagnostic.c $(SQUARE_CORE) -Wl,-Map,build/interpreted-square$(if $(SELECTOR),$(SELECTOR),0).map -o build/interpreted-square$(if $(SELECTOR),$(SELECTOR),0).prg
vice-test: c64
	python tools/interpreted_square_vice_test.py --phase $(if $(SELECTOR),$(SELECTOR),0) --vice $(VICE) --rom-dir $(C64_ROM_DIR) --data-dir $(VICE_DATA_DIR)
.PHONY: check-host c64 vice-test
