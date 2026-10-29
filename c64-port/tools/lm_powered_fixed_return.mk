# SPDX-License-Identifier: MIT
SHELL = /bin/bash
.SHELLFLAGS = -eu -o pipefail -c
LM_FR_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
LM_FR_CORE = src/lm_powered_fixed_return.c src/agc_word.c tests/lm_powered_fixed_return_cases.c tests/lm_powered_interface_cases.c tests/lm_powered_axis_cases.c tests/lm_powered_trig_cases.c
all: build/lm-powered-fixed-return-native build/lm-powered-fixed-return-reference
build/lm-powered-fixed-return-native: tools/lm_powered_fixed_return_snapshot.c $(LM_FR_CORE)
	gcc -O2 $(LM_FR_FLAGS) tools/lm_powered_fixed_return_snapshot.c $(LM_FR_CORE) -o $@
build/lm-powered-fixed-return-reference: tools/lm_powered_fixed_return_reference.c tests/lm_powered_fixed_return_cases.c tests/lm_powered_interface_cases.c tests/lm_powered_axis_cases.c tests/lm_powered_trig_cases.c
	gcc -O2 -std=c99 -Iinclude -Itests -Ideps/virtualagc/yaAGC tools/lm_powered_fixed_return_reference.c tests/lm_powered_fixed_return_cases.c tests/lm_powered_interface_cases.c tests/lm_powered_axis_cases.c tests/lm_powered_trig_cases.c tests/cm_executive_cases.c src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	build/lm-powered-fixed-return-reference deps/lm-powered-fixture/MAIN.agc.bin 2> build/lm-powered-fixed-return-reference.trace | gzip -n -9 > build/lm-powered-fixed-return-reference.snapshots.gz
	build/lm-powered-fixed-return-native | gzip -n -9 > build/lm-powered-fixed-return-native.snapshots.gz
	cmp build/lm-powered-fixed-return-reference.snapshots.gz build/lm-powered-fixed-return-native.snapshots.gz
sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(LM_FR_FLAGS) tools/lm_powered_fixed_return_snapshot.c $(LM_FR_CORE) -o build/lm-powered-fixed-return-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/lm-powered-fixed-return-sanitize | gzip -n -9 > build/lm-powered-fixed-return-sanitize.snapshots.gz
	cmp build/lm-powered-fixed-return-reference.snapshots.gz build/lm-powered-fixed-return-sanitize.snapshots.gz
.PHONY: all test sanitize


check-host:
	gcc -O2 $(LM_FR_FLAGS) tests/test_lm_powered_fixed_return.c tests/frozen_words.c $(LM_FR_CORE) -o build/test-lm-powered-fixed-return
	build/test-lm-powered-fixed-return
c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DLM_POWERED_FIXED_RETURN_CHECK_ONLY -DLM_POWERED_FIXED_RETURN_SELECTED=$(if $(SELECTOR),$(SELECTOR),0) $(LM_FR_FLAGS) tests/test_lm_powered_fixed_return.c tests/frozen_words.c tools/lm_powered_fixed_return_diagnostic.c $(LM_FR_CORE) -Wl,-Map,build/lm-powered-fixed-return$(if $(SELECTOR),$(SELECTOR),0).map -o build/lm-powered-fixed-return$(if $(SELECTOR),$(SELECTOR),0).prg
vice-test: c64
	python tools/lm_powered_fixed_return_vice_test.py --phase $(if $(SELECTOR),$(SELECTOR),0) --vice $(VICE) --rom-dir $(C64_ROM_DIR) --data-dir $(VICE_DATA_DIR)
test-lm-powered-fixed-return: test check-host
.PHONY: check-host c64 vice-test test-lm-powered-fixed-return
