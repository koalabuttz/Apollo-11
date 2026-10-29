# SPDX-License-Identifier: MIT
SHELL = /bin/bash
.SHELLFLAGS = -eu -o pipefail -c
CM_PT_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
CM_PT_CORE = src/cm_powered_trig.c src/guidance_trig.c src/guidance_product.c src/agc_word.c src/agc_multiword.c tests/cm_powered_trig_cases.c
all: build/cm-powered-trig-native build/cm-powered-trig-reference
build/cm-powered-trig-native: tools/cm_powered_trig_snapshot.c $(CM_PT_CORE) include/cm_powered_trig.h tests/cm_powered_trig_cases.h
	gcc -O2 $(CM_PT_FLAGS) tools/cm_powered_trig_snapshot.c $(CM_PT_CORE) -o $@
build/cm-powered-trig-reference: tools/cm_powered_trig_reference.c tests/cm_powered_trig_cases.c tests/cm_powered_trig_cases.h
	gcc -O2 -std=c99 -Iinclude -Itests -Ideps/virtualagc/yaAGC tools/cm_powered_trig_reference.c tests/cm_powered_trig_cases.c tests/cm_executive_cases.c src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	build/cm-powered-trig-reference deps/cm-powered-fixture/MAIN.agc.bin | gzip -n -9 > build/cm-powered-trig-reference.snapshots.gz
	build/cm-powered-trig-native | gzip -n -9 > build/cm-powered-trig-native.snapshots.gz
	cmp build/cm-powered-trig-reference.snapshots.gz build/cm-powered-trig-native.snapshots.gz
sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CM_PT_FLAGS) tools/cm_powered_trig_snapshot.c $(CM_PT_CORE) -o build/cm-powered-trig-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/cm-powered-trig-sanitize | gzip -n -9 > build/cm-powered-trig-sanitize.snapshots.gz
	cmp build/cm-powered-trig-reference.snapshots.gz build/cm-powered-trig-sanitize.snapshots.gz
.PHONY: all test sanitize

check-host:
	gcc -O2 $(CM_PT_FLAGS) tests/test_cm_powered_trig.c tests/frozen_words.c $(CM_PT_CORE) -o build/test-cm-powered-trig
	build/test-cm-powered-trig
c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DCM_POWERED_TRIG_CHECK_ONLY -DCM_POWERED_TRIG_SELECTED=$(if $(SELECTOR),$(SELECTOR),0) $(CM_PT_FLAGS) tests/test_cm_powered_trig.c tests/frozen_words.c tools/cm_powered_trig_diagnostic.c $(CM_PT_CORE) -Wl,-Map,build/cm-powered-trig$(if $(SELECTOR),$(SELECTOR),0).map -o build/cm-powered-trig$(if $(SELECTOR),$(SELECTOR),0).prg
vice-test: c64
	python tools/cm_powered_trig_vice_test.py --phase $(if $(SELECTOR),$(SELECTOR),0) --vice $(VICE) --rom-dir $(C64_ROM_DIR) --data-dir $(VICE_DATA_DIR)
test-cm-powered-trig: test check-host
.PHONY: check-host c64 vice-test test-cm-powered-trig
