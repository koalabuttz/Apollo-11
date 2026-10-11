# SPDX-License-Identifier: MIT
LM_ST_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
LM_ST_CORE = src/lm_single_trig.c src/agc_word.c src/agc_multiword.c tests/lm_single_trig_cases.c
all: build/lm-single-trig-native build/lm-single-trig-reference
build/lm-single-trig-native: tools/lm_single_trig_snapshot.c $(LM_ST_CORE)
	gcc -O2 $(LM_ST_FLAGS) tools/lm_single_trig_snapshot.c $(LM_ST_CORE) -o $@
build/lm-single-trig-reference: tools/lm_single_trig_reference.c tests/lm_single_trig_cases.c
	gcc -O2 -std=c99 -Iinclude -Itests -Ideps/virtualagc/yaAGC tools/lm_single_trig_reference.c tests/lm_single_trig_cases.c tests/scenarios.c src/executive.c src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	build/lm-single-trig-reference deps/lm-single-trig-fixture/MAIN.agc.bin > build/lm-single-trig-reference.snapshots
	build/lm-single-trig-native > build/lm-single-trig-native.snapshots
	cmp build/lm-single-trig-reference.snapshots build/lm-single-trig-native.snapshots
sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(LM_ST_FLAGS) tools/lm_single_trig_snapshot.c $(LM_ST_CORE) -o build/lm-single-trig-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/lm-single-trig-sanitize > build/lm-single-trig-sanitize.snapshots
	cmp build/lm-single-trig-reference.snapshots build/lm-single-trig-sanitize.snapshots
.PHONY: all test sanitize

check-host:
	gcc -O2 $(LM_ST_FLAGS) tests/test_lm_single_trig.c tests/frozen_words.c $(LM_ST_CORE) -o build/test-lm-single-trig
	build/test-lm-single-trig
c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DLM_SINGLE_TRIG_CHECK_ONLY -DLM_SINGLE_TRIG_SELECTED=$(if $(SELECTOR),$(SELECTOR),0) $(LM_ST_FLAGS) tests/test_lm_single_trig.c tests/frozen_words.c tools/lm_single_trig_diagnostic.c $(LM_ST_CORE) -Wl,-Map,build/lm-single-trig$(if $(SELECTOR),$(SELECTOR),0).map -o build/lm-single-trig$(if $(SELECTOR),$(SELECTOR),0).prg
vice-test: c64
	python tools/lm_single_trig_vice_test.py --phase $(if $(SELECTOR),$(SELECTOR),0) --vice $(VICE) --rom-dir $(C64_ROM_DIR) --data-dir $(VICE_DATA_DIR)
test-lm-single-trig: test check-host
.PHONY: check-host c64 vice-test test-lm-single-trig
