# SPDX-License-Identifier: MIT
SHELL = /bin/bash
.SHELLFLAGS = -eu -o pipefail -c
INVERSE_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
INVERSE_CORE = src/inverse_trig.c src/guidance_norm.c src/guidance_product.c src/agc_word.c src/agc_multiword.c tests/inverse_trig_sweep_cases.c
all: build/inverse-trig-sweep-native build/inverse-trig-sweep-reference
build/inverse-trig-sweep-native: tools/inverse_trig_snapshot.c $(INVERSE_CORE)
	gcc -O2 $(INVERSE_FLAGS) tools/inverse_trig_snapshot.c $(INVERSE_CORE) -o $@
build/inverse-trig-sweep-reference: tools/inverse_trig_reference.c tests/inverse_trig_sweep_cases.c
	gcc -O2 -std=c99 -Iinclude -Itests -Ideps/virtualagc/yaAGC tools/inverse_trig_reference.c tests/inverse_trig_sweep_cases.c tests/cm_executive_cases.c src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	build/inverse-trig-sweep-reference deps/lm-powered-fixture/MAIN.agc.bin 0 2> build/inverse-trig-sweep-lm-reference.trace | gzip -n -9 > build/inverse-trig-sweep-lm-reference.snapshots.gz
	build/inverse-trig-sweep-native 0 | gzip -n -9 > build/inverse-trig-sweep-lm-native.snapshots.gz
	cmp build/inverse-trig-sweep-lm-reference.snapshots.gz build/inverse-trig-sweep-lm-native.snapshots.gz
	build/inverse-trig-sweep-reference deps/cm-powered-fixture/MAIN.agc.bin 1 2> build/inverse-trig-sweep-cm-reference.trace | gzip -n -9 > build/inverse-trig-sweep-cm-reference.snapshots.gz
	build/inverse-trig-sweep-native 1 | gzip -n -9 > build/inverse-trig-sweep-cm-native.snapshots.gz
	cmp build/inverse-trig-sweep-cm-reference.snapshots.gz build/inverse-trig-sweep-cm-native.snapshots.gz
sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(INVERSE_FLAGS) tools/inverse_trig_snapshot.c $(INVERSE_CORE) -o build/inverse-trig-sweep-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/inverse-trig-sweep-sanitize 0 | gzip -n -9 > build/inverse-trig-sweep-lm-sanitize.snapshots.gz
	cmp build/inverse-trig-sweep-lm-reference.snapshots.gz build/inverse-trig-sweep-lm-sanitize.snapshots.gz
	ASAN_OPTIONS=detect_leaks=0 build/inverse-trig-sweep-sanitize 1 | gzip -n -9 > build/inverse-trig-sweep-cm-sanitize.snapshots.gz
	cmp build/inverse-trig-sweep-cm-reference.snapshots.gz build/inverse-trig-sweep-cm-sanitize.snapshots.gz
.PHONY: all test sanitize
check-host:
	gcc -O2 $(INVERSE_FLAGS) tests/test_inverse_trig_sweep.c tests/frozen_words.c $(INVERSE_CORE) -o build/test-inverse-trig-sweep
	build/test-inverse-trig-sweep
c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DINVERSE_TRIG_CHECK_ONLY -DINVERSE_TRIG_SELECTED=$(if $(SELECTOR),$(SELECTOR),0) $(INVERSE_FLAGS) tests/test_inverse_trig_sweep.c tests/frozen_words.c tools/inverse_trig_diagnostic.c $(INVERSE_CORE) -Wl,-Map,build/inverse-trig-sweep$(if $(SELECTOR),$(SELECTOR),0).map -o build/inverse-trig-sweep$(if $(SELECTOR),$(SELECTOR),0).prg
vice-test: c64
	python tools/inverse_trig_sweep_vice_test.py --phase $(if $(SELECTOR),$(SELECTOR),0) --vice $(VICE) --rom-dir $(C64_ROM_DIR) --data-dir $(VICE_DATA_DIR)
.PHONY: check-host c64 vice-test
