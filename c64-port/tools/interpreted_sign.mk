# SPDX-License-Identifier: MIT
SHELL = /bin/bash
.SHELLFLAGS = -eu -o pipefail -c
SIGN_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
SIGN_CORE = src/interpreted_sign.c src/agc_word.c src/agc_multiword.c tests/interpreted_sign_cases.c
all: build/interpreted-sign-native build/interpreted-sign-reference
build/interpreted-sign-native: tools/interpreted_sign_snapshot.c $(SIGN_CORE)
	gcc -O2 $(SIGN_FLAGS) tools/interpreted_sign_snapshot.c $(SIGN_CORE) -o $@
build/interpreted-sign-reference: tools/interpreted_sign_reference.c tests/interpreted_sign_cases.c
	gcc -O2 -std=c99 -Iinclude -Itests -Ideps/virtualagc/yaAGC tools/interpreted_sign_reference.c tests/interpreted_sign_cases.c tests/cm_executive_cases.c src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	build/interpreted-sign-reference deps/lm-powered-fixture/MAIN.agc.bin 0 2> build/interpreted-sign-lm-reference.trace | gzip -n -9 > build/interpreted-sign-lm-reference.snapshots.gz
	build/interpreted-sign-native 0 | gzip -n -9 > build/interpreted-sign-lm-native.snapshots.gz
	cmp build/interpreted-sign-lm-reference.snapshots.gz build/interpreted-sign-lm-native.snapshots.gz
	build/interpreted-sign-reference deps/cm-powered-fixture/MAIN.agc.bin 1 2> build/interpreted-sign-cm-reference.trace | gzip -n -9 > build/interpreted-sign-cm-reference.snapshots.gz
	build/interpreted-sign-native 1 | gzip -n -9 > build/interpreted-sign-cm-native.snapshots.gz
	cmp build/interpreted-sign-cm-reference.snapshots.gz build/interpreted-sign-cm-native.snapshots.gz
sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(SIGN_FLAGS) tools/interpreted_sign_snapshot.c $(SIGN_CORE) -o build/interpreted-sign-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/interpreted-sign-sanitize 0 | gzip -n -9 > build/interpreted-sign-lm-sanitize.snapshots.gz
	cmp build/interpreted-sign-lm-reference.snapshots.gz build/interpreted-sign-lm-sanitize.snapshots.gz
	ASAN_OPTIONS=detect_leaks=0 build/interpreted-sign-sanitize 1 | gzip -n -9 > build/interpreted-sign-cm-sanitize.snapshots.gz
	cmp build/interpreted-sign-cm-reference.snapshots.gz build/interpreted-sign-cm-sanitize.snapshots.gz
.PHONY: all test sanitize
check-host:
	gcc -O2 $(SIGN_FLAGS) tests/test_interpreted_sign.c tests/frozen_words.c $(SIGN_CORE) -o build/test-interpreted-sign
	build/test-interpreted-sign
c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DINTERPRETED_SIGN_CHECK_ONLY -DINTERPRETED_SIGN_SELECTED=$(if $(SELECTOR),$(SELECTOR),0) $(SIGN_FLAGS) tests/test_interpreted_sign.c tests/frozen_words.c tools/interpreted_sign_diagnostic.c $(SIGN_CORE) -Wl,-Map,build/interpreted-sign$(if $(SELECTOR),$(SELECTOR),0).map -o build/interpreted-sign$(if $(SELECTOR),$(SELECTOR),0).prg
vice-test: c64
	python tools/interpreted_sign_vice_test.py --phase $(if $(SELECTOR),$(SELECTOR),0) --vice $(VICE) --rom-dir $(C64_ROM_DIR) --data-dir $(VICE_DATA_DIR)
.PHONY: check-host c64 vice-test
