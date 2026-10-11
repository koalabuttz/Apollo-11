# SPDX-License-Identifier: MIT
SHELL = /bin/bash
.SHELLFLAGS = -eu -o pipefail -c
LM_FP_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
LM_FP_CORE = src/lm_fleshpot.c src/guidance_product.c src/agc_word.c src/agc_multiword.c tests/lm_fleshpot_cases.c tests/lm_powered_axis_cases.c tests/lm_powered_trig_cases.c
all: build/lm-fleshpot-native build/lm-fleshpot-reference
build/lm-fleshpot-native: tools/lm_fleshpot_snapshot.c $(LM_FP_CORE)
	gcc -O2 $(LM_FP_FLAGS) tools/lm_fleshpot_snapshot.c $(LM_FP_CORE) -o $@
build/lm-fleshpot-reference: tools/lm_fleshpot_reference.c tests/lm_fleshpot_cases.c tests/lm_powered_axis_cases.c tests/lm_powered_trig_cases.c
	gcc -O2 -std=c99 -Iinclude -Itests -Ideps/virtualagc/yaAGC tools/lm_fleshpot_reference.c tests/lm_fleshpot_cases.c tests/lm_powered_axis_cases.c tests/lm_powered_trig_cases.c tests/cm_executive_cases.c src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	build/lm-fleshpot-reference deps/lm-powered-fixture/MAIN.agc.bin | gzip -n -9 > build/lm-fleshpot-reference.snapshots.gz
	build/lm-fleshpot-native | gzip -n -9 > build/lm-fleshpot-native.snapshots.gz
	cmp build/lm-fleshpot-reference.snapshots.gz build/lm-fleshpot-native.snapshots.gz
sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(LM_FP_FLAGS) tools/lm_fleshpot_snapshot.c $(LM_FP_CORE) -o build/lm-fleshpot-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/lm-fleshpot-sanitize | gzip -n -9 > build/lm-fleshpot-sanitize.snapshots.gz
	cmp build/lm-fleshpot-reference.snapshots.gz build/lm-fleshpot-sanitize.snapshots.gz
.PHONY: all test sanitize

check-host:
	gcc -O2 $(LM_FP_FLAGS) tests/test_lm_fleshpot.c tests/frozen_words.c $(LM_FP_CORE) -o build/test-lm-fleshpot
	build/test-lm-fleshpot
c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DLM_FLESHPOT_CHECK_ONLY -DLM_FLESHPOT_SELECTED=$(if $(SELECTOR),$(SELECTOR),0) $(LM_FP_FLAGS) tests/test_lm_fleshpot.c tests/frozen_words.c tools/lm_fleshpot_diagnostic.c $(LM_FP_CORE) -Wl,-Map,build/lm-fleshpot$(if $(SELECTOR),$(SELECTOR),0).map -o build/lm-fleshpot$(if $(SELECTOR),$(SELECTOR),0).prg
vice-test: c64
	python tools/lm_fleshpot_vice_test.py --phase $(if $(SELECTOR),$(SELECTOR),0) --vice $(VICE) --rom-dir $(C64_ROM_DIR) --data-dir $(VICE_DATA_DIR)
test-lm-fleshpot: test check-host
.PHONY: check-host c64 vice-test test-lm-fleshpot
