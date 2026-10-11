# SPDX-License-Identifier: MIT
SHELL = /bin/bash
.SHELLFLAGS = -eu -o pipefail -c
LM_PI_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
LM_PI_CORE = src/lm_powered_interface.c src/lm_powered_axis.c src/lm_powered_trig.c src/guidance_trig.c src/guidance_rotation.c src/guidance_product.c src/agc_word.c src/agc_multiword.c tests/lm_powered_interface_cases.c tests/lm_powered_axis_cases.c tests/lm_powered_trig_cases.c
all: build/lm-powered-interface-native build/lm-powered-interface-reference
build/lm-powered-interface-native: tools/lm_powered_interface_snapshot.c $(LM_PI_CORE)
	gcc -O2 $(LM_PI_FLAGS) tools/lm_powered_interface_snapshot.c $(LM_PI_CORE) -o $@
build/lm-powered-interface-reference: tools/lm_powered_interface_reference.c tests/lm_powered_interface_cases.c tests/lm_powered_axis_cases.c tests/lm_powered_trig_cases.c
	gcc -O2 -std=c99 -Iinclude -Itests -Ideps/virtualagc/yaAGC tools/lm_powered_interface_reference.c tests/lm_powered_interface_cases.c tests/lm_powered_axis_cases.c tests/lm_powered_trig_cases.c tests/cm_executive_cases.c src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	build/lm-powered-interface-reference deps/lm-powered-fixture/MAIN.agc.bin 2> build/lm-powered-interface-reference.trace | gzip -n -9 > build/lm-powered-interface-reference.snapshots.gz
	build/lm-powered-interface-native | gzip -n -9 > build/lm-powered-interface-native.snapshots.gz
	cmp build/lm-powered-interface-reference.snapshots.gz build/lm-powered-interface-native.snapshots.gz
sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(LM_PI_FLAGS) tools/lm_powered_interface_snapshot.c $(LM_PI_CORE) -o build/lm-powered-interface-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/lm-powered-interface-sanitize | gzip -n -9 > build/lm-powered-interface-sanitize.snapshots.gz
	cmp build/lm-powered-interface-reference.snapshots.gz build/lm-powered-interface-sanitize.snapshots.gz
.PHONY: all test sanitize

check-host:
	gcc -O2 $(LM_PI_FLAGS) tests/test_lm_powered_interface.c tests/frozen_words.c $(LM_PI_CORE) -o build/test-lm-powered-interface
	build/test-lm-powered-interface
c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DLM_POWERED_INTERFACE_CHECK_ONLY -DLM_POWERED_INTERFACE_SELECTED=$(if $(SELECTOR),$(SELECTOR),0) $(LM_PI_FLAGS) tests/test_lm_powered_interface.c tests/frozen_words.c tools/lm_powered_interface_diagnostic.c $(LM_PI_CORE) -Wl,-Map,build/lm-powered-interface$(if $(SELECTOR),$(SELECTOR),0).map -o build/lm-powered-interface$(if $(SELECTOR),$(SELECTOR),0).prg
vice-test: c64
	python tools/lm_powered_interface_vice_test.py --phase $(if $(SELECTOR),$(SELECTOR),0) --vice $(VICE) --rom-dir $(C64_ROM_DIR) --data-dir $(VICE_DATA_DIR)
test-lm-powered-interface: test check-host
.PHONY: check-host c64 vice-test test-lm-powered-interface
