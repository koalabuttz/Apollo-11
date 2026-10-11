# SPDX-License-Identifier: MIT
CM_SR_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
CM_SR_CORE = src/cm_single_return.c src/agc_word.c tests/cm_single_return_cases.c tests/cm_trig_cases.c
all: build/cm-single-return-native build/cm-single-return-reference
build/cm-single-return-native: tools/cm_single_return_snapshot.c $(CM_SR_CORE)
	gcc -O2 $(CM_SR_FLAGS) tools/cm_single_return_snapshot.c $(CM_SR_CORE) -o $@
build/cm-single-return-reference: tools/cm_single_return_reference.c $(CM_SR_CORE)
	gcc -O2 -std=c99 -Iinclude -Itests -Ideps/virtualagc/yaAGC tools/cm_single_return_reference.c tests/cm_single_return_cases.c tests/cm_trig_cases.c tests/cm_executive_cases.c src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	build/cm-single-return-reference deps/cm-trig-fixture/MAIN.agc.bin > build/cm-single-return-reference.snapshots
	build/cm-single-return-native < build/cm-single-return-reference.snapshots > build/cm-single-return-native.snapshots
	python tools/cm_single_return_compare.py
sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CM_SR_FLAGS) tools/cm_single_return_snapshot.c $(CM_SR_CORE) -o build/cm-single-return-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/cm-single-return-sanitize < build/cm-single-return-reference.snapshots > build/cm-single-return-sanitize.snapshots
	cmp build/cm-single-return-native.snapshots build/cm-single-return-sanitize.snapshots
.PHONY: all test sanitize
check-host:
	gcc -O2 $(CM_SR_FLAGS) tests/test_cm_single_return.c tests/frozen_words.c $(CM_SR_CORE) -o build/test-cm-single-return
	build/test-cm-single-return
CM_SR_SELECTED = $(shell expr $(if $(SELECTOR),$(SELECTOR),0) % 2)
CM_SR_COMPOSE = $(if $(filter 2 3,$(SELECTOR)),-DCM_SINGLE_RETURN_COMPLETE src/cm_quictrig.c src/agc_multiword.c,)
c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DCM_SINGLE_RETURN_CHECK_ONLY -DCM_SINGLE_RETURN_SELECTED=$(CM_SR_SELECTED) $(CM_SR_COMPOSE) $(CM_SR_FLAGS) tests/test_cm_single_return.c tests/frozen_words.c tools/cm_single_return_diagnostic.c $(CM_SR_CORE) -Wl,-Map,build/cm-single-return$(if $(SELECTOR),$(SELECTOR),0).map -o build/cm-single-return$(if $(SELECTOR),$(SELECTOR),0).prg
test-cm-single-return: test check-host
complete-test:
	gcc -O2 $(CM_SR_FLAGS) tools/cm_single_complete_snapshot.c src/cm_quictrig.c src/agc_multiword.c $(CM_SR_CORE) -o build/cm-single-complete-native
	build/cm-single-complete-native > build/cm-single-complete-native.snapshots
	cmp build/cm-single-return-native.snapshots build/cm-single-complete-native.snapshots
complete-sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CM_SR_FLAGS) tools/cm_single_complete_snapshot.c src/cm_quictrig.c src/agc_multiword.c $(CM_SR_CORE) -o build/cm-single-complete-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/cm-single-complete-sanitize > build/cm-single-complete-sanitize.snapshots
	cmp build/cm-single-return-native.snapshots build/cm-single-complete-sanitize.snapshots

complete-check-host:
	gcc -O2 $(CM_SR_FLAGS) -DCM_SINGLE_RETURN_COMPLETE tests/test_cm_single_return.c tests/frozen_words.c src/cm_quictrig.c src/agc_multiword.c $(CM_SR_CORE) -o build/test-cm-single-complete
	build/test-cm-single-complete
.PHONY: check-host c64 complete-test complete-sanitize complete-check-host test-cm-single-return

vice-test: c64
	python tools/cm_single_return_vice_test.py --phase $(if $(SELECTOR),$(SELECTOR),0) --vice $(VICE) --rom-dir $(C64_ROM_DIR) --data-dir $(VICE_DATA_DIR)
.PHONY: vice-test
