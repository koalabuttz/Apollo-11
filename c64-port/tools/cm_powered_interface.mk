# SPDX-License-Identifier: MIT
SHELL = /bin/bash
.SHELLFLAGS = -eu -o pipefail -c
CM_PI_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
CM_PI_CORE = src/cm_powered_interface.c src/cm_powered_axis.c src/cm_powered_trig.c src/guidance_trig.c src/cm_rotation.c src/guidance_product.c src/agc_word.c src/agc_multiword.c tests/cm_powered_interface_cases.c tests/cm_powered_axis_cases.c tests/cm_powered_trig_cases.c
all: build/cm-powered-interface-native build/cm-powered-interface-reference
build/cm-powered-interface-native: tools/cm_powered_interface_snapshot.c $(CM_PI_CORE)
	gcc -O2 $(CM_PI_FLAGS) tools/cm_powered_interface_snapshot.c $(CM_PI_CORE) -o $@
build/cm-powered-interface-reference: tools/cm_powered_interface_reference.c tests/cm_powered_interface_cases.c tests/cm_powered_axis_cases.c tests/cm_powered_trig_cases.c
	gcc -O2 -std=c99 -Iinclude -Itests -Ideps/virtualagc/yaAGC tools/cm_powered_interface_reference.c tests/cm_powered_interface_cases.c tests/cm_powered_axis_cases.c tests/cm_powered_trig_cases.c tests/cm_executive_cases.c src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	build/cm-powered-interface-reference deps/cm-powered-fixture/MAIN.agc.bin 2> build/cm-powered-interface-reference.trace | gzip -n -9 > build/cm-powered-interface-reference.snapshots.gz
	build/cm-powered-interface-native | gzip -n -9 > build/cm-powered-interface-native.snapshots.gz
	cmp build/cm-powered-interface-reference.snapshots.gz build/cm-powered-interface-native.snapshots.gz
sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CM_PI_FLAGS) tools/cm_powered_interface_snapshot.c $(CM_PI_CORE) -o build/cm-powered-interface-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/cm-powered-interface-sanitize | gzip -n -9 > build/cm-powered-interface-sanitize.snapshots.gz
	cmp build/cm-powered-interface-reference.snapshots.gz build/cm-powered-interface-sanitize.snapshots.gz
.PHONY: all test sanitize

check-host:
	gcc -O2 $(CM_PI_FLAGS) tests/test_cm_powered_interface.c tests/frozen_words.c $(CM_PI_CORE) -o build/test-cm-powered-interface
	build/test-cm-powered-interface
c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DCM_POWERED_INTERFACE_CHECK_ONLY -DCM_POWERED_INTERFACE_SELECTED=$(if $(SELECTOR),$(SELECTOR),0) $(CM_PI_FLAGS) tests/test_cm_powered_interface.c tests/frozen_words.c tools/cm_powered_interface_diagnostic.c $(CM_PI_CORE) -Wl,-Map,build/cm-powered-interface$(if $(SELECTOR),$(SELECTOR),0).map -o build/cm-powered-interface$(if $(SELECTOR),$(SELECTOR),0).prg
vice-test: c64
	python tools/cm_powered_interface_vice_test.py --phase $(if $(SELECTOR),$(SELECTOR),0) --vice $(VICE) --rom-dir $(C64_ROM_DIR) --data-dir $(VICE_DATA_DIR)
test-cm-powered-interface: test check-host
.PHONY: check-host c64 vice-test test-cm-powered-interface
