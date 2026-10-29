# SPDX-License-Identifier: MIT
SHELL = /bin/bash
.SHELLFLAGS = -eu -o pipefail -c
LM_PQ_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
LM_PQ_CORE = src/lm_powered_constants.c tests/test_lm_powered_constants.c src/lm_powered_quick.c src/lm_single_trig.c src/agc_word.c src/agc_multiword.c tests/lm_powered_quick_cases.c tests/lm_powered_trig_cases.c
all: build/lm-powered-quick-native build/lm-powered-quick-reference
build/lm-powered-quick-native: tools/lm_powered_quick_snapshot.c $(LM_PQ_CORE)
	gcc -O2 $(LM_PQ_FLAGS) tools/lm_powered_quick_snapshot.c $(LM_PQ_CORE) -o $@
build/lm-powered-quick-reference: tools/lm_powered_quick_reference.c tests/lm_powered_quick_cases.c tests/lm_powered_trig_cases.c
	gcc -O2 -std=c99 -Iinclude -Itests -Ideps/virtualagc/yaAGC tools/lm_powered_quick_reference.c tests/lm_powered_quick_cases.c tests/lm_powered_trig_cases.c tests/cm_executive_cases.c src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	build/lm-powered-quick-reference deps/lm-powered-fixture/MAIN.agc.bin | gzip -n -9 > build/lm-powered-quick-reference.snapshots.gz
	build/lm-powered-quick-native | gzip -n -9 > build/lm-powered-quick-native.snapshots.gz
	cmp build/lm-powered-quick-reference.snapshots.gz build/lm-powered-quick-native.snapshots.gz
sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(LM_PQ_FLAGS) tools/lm_powered_quick_snapshot.c $(LM_PQ_CORE) -o build/lm-powered-quick-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/lm-powered-quick-sanitize | gzip -n -9 > build/lm-powered-quick-sanitize.snapshots.gz
	cmp build/lm-powered-quick-reference.snapshots.gz build/lm-powered-quick-sanitize.snapshots.gz
.PHONY: all test sanitize


check-host:
	gcc -O2 $(LM_PQ_FLAGS) tests/test_lm_powered_quick.c tests/frozen_words.c $(LM_PQ_CORE) -o build/test-lm-powered-quick
	build/test-lm-powered-quick
c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DLM_POWERED_QUICK_CHECK_ONLY -DLM_POWERED_QUICK_SELECTED=$(if $(SELECTOR),$(SELECTOR),0) $(LM_PQ_FLAGS) tests/test_lm_powered_quick.c tests/frozen_words.c tools/lm_powered_quick_diagnostic.c $(LM_PQ_CORE) -Wl,-Map,build/lm-powered-quick$(if $(SELECTOR),$(SELECTOR),0).map -o build/lm-powered-quick$(if $(SELECTOR),$(SELECTOR),0).prg
vice-test: c64
	python tools/lm_powered_quick_vice_test.py --phase $(if $(SELECTOR),$(SELECTOR),0) --vice $(VICE) --rom-dir $(C64_ROM_DIR) --data-dir $(VICE_DATA_DIR)
test-lm-powered-quick: test check-host
.PHONY: check-host c64 vice-test test-lm-powered-quick
