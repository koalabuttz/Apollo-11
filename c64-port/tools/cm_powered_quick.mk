# SPDX-License-Identifier: MIT
SHELL = /bin/bash
.SHELLFLAGS = -eu -o pipefail -c
CM_PQ_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
CM_PQ_CORE = src/cm_powered_constants.c tests/test_cm_powered_constants.c src/cm_powered_quick_return.c src/cm_quictrig.c src/agc_word.c src/agc_multiword.c tests/cm_powered_quick_cases.c tests/cm_powered_trig_cases.c
all: build/cm-powered-quick-native build/cm-powered-quick-reference
build/cm-powered-quick-native: tools/cm_powered_quick_snapshot.c $(CM_PQ_CORE)
	gcc -O2 $(CM_PQ_FLAGS) tools/cm_powered_quick_snapshot.c $(CM_PQ_CORE) -o $@
build/cm-powered-quick-reference: tools/cm_powered_quick_reference.c tests/cm_powered_quick_cases.c tests/cm_powered_trig_cases.c
	gcc -O2 -std=c99 -Iinclude -Itests -Ideps/virtualagc/yaAGC tools/cm_powered_quick_reference.c tests/cm_powered_quick_cases.c tests/cm_powered_trig_cases.c tests/cm_executive_cases.c src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	build/cm-powered-quick-reference deps/cm-powered-fixture/MAIN.agc.bin | gzip -n -9 > build/cm-powered-quick-reference.snapshots.gz
	build/cm-powered-quick-native | gzip -n -9 > build/cm-powered-quick-native.snapshots.gz
	cmp build/cm-powered-quick-reference.snapshots.gz build/cm-powered-quick-native.snapshots.gz
sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CM_PQ_FLAGS) tools/cm_powered_quick_snapshot.c $(CM_PQ_CORE) -o build/cm-powered-quick-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/cm-powered-quick-sanitize | gzip -n -9 > build/cm-powered-quick-sanitize.snapshots.gz
	cmp build/cm-powered-quick-reference.snapshots.gz build/cm-powered-quick-sanitize.snapshots.gz
.PHONY: all test sanitize


check-host:
	gcc -O2 $(CM_PQ_FLAGS) tests/test_cm_powered_quick.c tests/frozen_words.c $(CM_PQ_CORE) -o build/test-cm-powered-quick
	build/test-cm-powered-quick
c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DCM_POWERED_QUICK_CHECK_ONLY -DCM_POWERED_QUICK_SELECTED=$(if $(SELECTOR),$(SELECTOR),0) $(CM_PQ_FLAGS) tests/test_cm_powered_quick.c tests/frozen_words.c tools/cm_powered_quick_diagnostic.c $(CM_PQ_CORE) -Wl,-Map,build/cm-powered-quick$(if $(SELECTOR),$(SELECTOR),0).map -o build/cm-powered-quick$(if $(SELECTOR),$(SELECTOR),0).prg
vice-test: c64
	python tools/cm_powered_quick_vice_test.py --phase $(if $(SELECTOR),$(SELECTOR),0) --vice $(VICE) --rom-dir $(C64_ROM_DIR) --data-dir $(VICE_DATA_DIR)
test-cm-powered-quick: test check-host
.PHONY: check-host c64 vice-test test-cm-powered-quick
