# SPDX-License-Identifier: MIT
SHELL = /bin/bash
.SHELLFLAGS = -eu -o pipefail -c
LM_PT_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
LM_PT_CORE = src/lm_powered_trig.c src/guidance_trig.c src/guidance_product.c src/agc_word.c src/agc_multiword.c tests/lm_powered_trig_cases.c
all: build/lm-powered-trig-native build/lm-powered-trig-reference
build/lm-powered-trig-native: tools/lm_powered_trig_snapshot.c $(LM_PT_CORE) include/lm_powered_trig.h tests/lm_powered_trig_cases.h
	gcc -O2 $(LM_PT_FLAGS) tools/lm_powered_trig_snapshot.c $(LM_PT_CORE) -o $@
build/lm-powered-trig-reference: tools/lm_powered_trig_reference.c tests/lm_powered_trig_cases.c tests/lm_powered_trig_cases.h
	gcc -O2 -std=c99 -Iinclude -Itests -Ideps/virtualagc/yaAGC tools/lm_powered_trig_reference.c tests/lm_powered_trig_cases.c tests/cm_executive_cases.c src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	build/lm-powered-trig-reference deps/lm-powered-fixture/MAIN.agc.bin 2> build/lm-powered-trig-reference.trace | gzip -n -9 > build/lm-powered-trig-reference.snapshots.gz
	build/lm-powered-trig-native | gzip -n -9 > build/lm-powered-trig-native.snapshots.gz
	cmp build/lm-powered-trig-reference.snapshots.gz build/lm-powered-trig-native.snapshots.gz
sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(LM_PT_FLAGS) tools/lm_powered_trig_snapshot.c $(LM_PT_CORE) -o build/lm-powered-trig-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/lm-powered-trig-sanitize | gzip -n -9 > build/lm-powered-trig-sanitize.snapshots.gz
	cmp build/lm-powered-trig-reference.snapshots.gz build/lm-powered-trig-sanitize.snapshots.gz
.PHONY: all test sanitize

check-host:
	gcc -O2 $(LM_PT_FLAGS) tests/test_lm_powered_trig.c tests/frozen_words.c $(LM_PT_CORE) -o build/test-lm-powered-trig
	build/test-lm-powered-trig
c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DLM_POWERED_TRIG_CHECK_ONLY -DLM_POWERED_TRIG_SELECTED=$(if $(SELECTOR),$(SELECTOR),0) $(LM_PT_FLAGS) tests/test_lm_powered_trig.c tests/frozen_words.c tools/lm_powered_trig_diagnostic.c $(LM_PT_CORE) -Wl,-Map,build/lm-powered-trig$(if $(SELECTOR),$(SELECTOR),0).map -o build/lm-powered-trig$(if $(SELECTOR),$(SELECTOR),0).prg
vice-test: c64
	python tools/lm_powered_trig_vice_test.py --phase $(if $(SELECTOR),$(SELECTOR),0) --vice $(VICE) --rom-dir $(C64_ROM_DIR) --data-dir $(VICE_DATA_DIR)
test-lm-powered-trig: test check-host
.PHONY: check-host c64 vice-test test-lm-powered-trig
