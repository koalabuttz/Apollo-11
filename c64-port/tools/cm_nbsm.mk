# SPDX-License-Identifier: MIT
SHELL = /bin/bash
.SHELLFLAGS = -eu -o pipefail -c
CM_NB_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
CM_NB_CORE = src/cm_nbsm.c src/cm_powered_interface.c src/cm_powered_axis.c src/cm_powered_trig.c src/guidance_trig.c src/cm_rotation.c src/guidance_product.c src/agc_word.c src/agc_multiword.c tests/cm_nbsm_cases.c tests/cm_powered_interface_cases.c tests/cm_powered_axis_cases.c tests/cm_powered_trig_cases.c
all: build/cm-nbsm-native build/cm-nbsm-reference
build/cm-nbsm-native: tools/cm_nbsm_snapshot.c $(CM_NB_CORE)
	gcc -O2 $(CM_NB_FLAGS) tools/cm_nbsm_snapshot.c $(CM_NB_CORE) -o $@
build/cm-nbsm-reference: tools/cm_nbsm_reference.c tests/cm_nbsm_cases.c tests/cm_powered_interface_cases.c tests/cm_powered_axis_cases.c tests/cm_powered_trig_cases.c
	gcc -O2 -std=c99 -Iinclude -Itests -Ideps/virtualagc/yaAGC tools/cm_nbsm_reference.c tests/cm_nbsm_cases.c tests/cm_powered_interface_cases.c tests/cm_powered_axis_cases.c tests/cm_powered_trig_cases.c tests/cm_executive_cases.c src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	build/cm-nbsm-reference deps/cm-powered-fixture/MAIN.agc.bin 2> build/cm-nbsm-reference.trace | gzip -n -9 > build/cm-nbsm-reference.snapshots.gz
	build/cm-nbsm-native | gzip -n -9 > build/cm-nbsm-native.snapshots.gz
	cmp build/cm-nbsm-reference.snapshots.gz build/cm-nbsm-native.snapshots.gz
sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CM_NB_FLAGS) tools/cm_nbsm_snapshot.c $(CM_NB_CORE) -o build/cm-nbsm-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/cm-nbsm-sanitize | gzip -n -9 > build/cm-nbsm-sanitize.snapshots.gz
	cmp build/cm-nbsm-reference.snapshots.gz build/cm-nbsm-sanitize.snapshots.gz
.PHONY: all test sanitize


check-host:
	gcc -O2 $(CM_NB_FLAGS) tests/test_cm_nbsm.c tests/frozen_words.c $(CM_NB_CORE) -o build/test-cm-nbsm
	build/test-cm-nbsm
c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DCM_NBSM_CHECK_ONLY -DCM_NBSM_SELECTED=$(if $(SELECTOR),$(SELECTOR),0) $(CM_NB_FLAGS) tests/test_cm_nbsm.c tests/frozen_words.c tools/cm_nbsm_diagnostic.c $(CM_NB_CORE) -Wl,-Map,build/cm-nbsm$(if $(SELECTOR),$(SELECTOR),0).map -o build/cm-nbsm$(if $(SELECTOR),$(SELECTOR),0).prg
vice-test: c64
	python tools/cm_nbsm_vice_test.py --phase $(if $(SELECTOR),$(SELECTOR),0) --vice $(VICE) --rom-dir $(C64_ROM_DIR) --data-dir $(VICE_DATA_DIR)
test-cm-nbsm: test check-host
.PHONY: check-host c64 vice-test test-cm-nbsm
