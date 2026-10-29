# SPDX-License-Identifier: MIT
SHELL = /bin/bash
.SHELLFLAGS = -eu -o pipefail -c
CM_MY_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
CM_MY_CORE = src/cm_mysubs.c src/agc_word.c src/agc_multiword.c tests/cm_mysubs_cases.c
all: build/cm-mysubs-native build/cm-mysubs-reference
build/cm-mysubs-native: tools/cm_mysubs_snapshot.c $(CM_MY_CORE)
	gcc -O2 $(CM_MY_FLAGS) tools/cm_mysubs_snapshot.c $(CM_MY_CORE) -o $@
build/cm-mysubs-reference: tools/cm_mysubs_reference.c tests/cm_mysubs_cases.c
	gcc -O2 -std=c99 -Iinclude -Itests -Ideps/virtualagc/yaAGC tools/cm_mysubs_reference.c tests/cm_mysubs_cases.c tests/cm_executive_cases.c src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	build/cm-mysubs-reference deps/cm-mysubs-fixture/MAIN.agc.bin 2> build/cm-mysubs-reference.trace | gzip -n -9 > build/cm-mysubs-reference.snapshots.gz
	build/cm-mysubs-native | gzip -n -9 > build/cm-mysubs-native.snapshots.gz
	cmp build/cm-mysubs-reference.snapshots.gz build/cm-mysubs-native.snapshots.gz
sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CM_MY_FLAGS) tools/cm_mysubs_snapshot.c $(CM_MY_CORE) -o build/cm-mysubs-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/cm-mysubs-sanitize | gzip -n -9 > build/cm-mysubs-sanitize.snapshots.gz
	cmp build/cm-mysubs-reference.snapshots.gz build/cm-mysubs-sanitize.snapshots.gz
.PHONY: all test sanitize

check-host:
	gcc -O2 $(CM_MY_FLAGS) tests/test_cm_mysubs.c tests/frozen_words.c $(CM_MY_CORE) -o build/test-cm-mysubs
	build/test-cm-mysubs
c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DCM_MYSUBS_CHECK_ONLY -DCM_MYSUBS_SELECTED=$(if $(SELECTOR),$(SELECTOR),0) $(CM_MY_FLAGS) tests/test_cm_mysubs.c tests/frozen_words.c tools/cm_mysubs_diagnostic.c $(CM_MY_CORE) -Wl,-Map,build/cm-mysubs$(if $(SELECTOR),$(SELECTOR),0).map -o build/cm-mysubs$(if $(SELECTOR),$(SELECTOR),0).prg
vice-test: c64
	python tools/cm_mysubs_vice_test.py --phase $(if $(SELECTOR),$(SELECTOR),0) --vice $(VICE) --rom-dir $(C64_ROM_DIR) --data-dir $(VICE_DATA_DIR)
test-cm-mysubs: test check-host
.PHONY: check-host c64 vice-test test-cm-mysubs
