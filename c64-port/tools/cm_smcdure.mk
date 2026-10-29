# SPDX-License-Identifier: MIT
SHELL = /bin/bash
.SHELLFLAGS = -eu -o pipefail -c
CM_SC_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
CM_SC_CORE = src/cm_smcdure.c src/guidance_product.c src/guidance_dp_divide.c src/guidance_sum.c src/guidance_shift.c src/agc_word.c src/agc_multiword.c tests/cm_smcdure_cases.c tests/cm_powered_interface_cases.c tests/cm_powered_axis_cases.c tests/cm_powered_trig_cases.c
all: build/cm-smcdure-native build/cm-smcdure-reference
build/cm-smcdure-native: tools/cm_smcdure_snapshot.c $(CM_SC_CORE)
	gcc -O2 $(CM_SC_FLAGS) tools/cm_smcdure_snapshot.c $(CM_SC_CORE) -o $@
build/cm-smcdure-reference: tools/cm_smcdure_reference.c tests/cm_smcdure_cases.c tests/cm_powered_interface_cases.c tests/cm_powered_axis_cases.c tests/cm_powered_trig_cases.c
	gcc -O2 -std=c99 -Iinclude -Itests -Ideps/virtualagc/yaAGC tools/cm_smcdure_reference.c tests/cm_smcdure_cases.c tests/cm_powered_interface_cases.c tests/cm_powered_axis_cases.c tests/cm_powered_trig_cases.c tests/cm_executive_cases.c src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	build/cm-smcdure-reference deps/cm-powered-fixture/MAIN.agc.bin 2> build/cm-smcdure-reference.trace | gzip -n -9 > build/cm-smcdure-reference.snapshots.gz
	build/cm-smcdure-native | gzip -n -9 > build/cm-smcdure-native.snapshots.gz
	cmp build/cm-smcdure-reference.snapshots.gz build/cm-smcdure-native.snapshots.gz
sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CM_SC_FLAGS) tools/cm_smcdure_snapshot.c $(CM_SC_CORE) -o build/cm-smcdure-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/cm-smcdure-sanitize | gzip -n -9 > build/cm-smcdure-sanitize.snapshots.gz
	cmp build/cm-smcdure-reference.snapshots.gz build/cm-smcdure-sanitize.snapshots.gz
.PHONY: all test sanitize

check-host:
	gcc -O2 $(CM_SC_FLAGS) tests/test_cm_smcdure.c tests/frozen_words.c $(CM_SC_CORE) -o build/test-cm-smcdure
	build/test-cm-smcdure
c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DCM_SMCDURE_CHECK_ONLY -DCM_SMCDURE_SELECTED=$(if $(SELECTOR),$(SELECTOR),0) $(CM_SC_FLAGS) tests/test_cm_smcdure.c tests/frozen_words.c tools/cm_smcdure_diagnostic.c $(CM_SC_CORE) -Wl,-Map,build/cm-smcdure$(if $(SELECTOR),$(SELECTOR),0).map -o build/cm-smcdure$(if $(SELECTOR),$(SELECTOR),0).prg
vice-test: c64
	python tools/cm_smcdure_vice_test.py --phase $(if $(SELECTOR),$(SELECTOR),0) --vice $(VICE) --rom-dir $(C64_ROM_DIR) --data-dir $(VICE_DATA_DIR)
test-cm-smcdure: test check-host
.PHONY: check-host c64 vice-test test-cm-smcdure
