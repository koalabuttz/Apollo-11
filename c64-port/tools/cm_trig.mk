# SPDX-License-Identifier: MIT
CM_TRIG_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
CM_TRIG_CORE = src/cm_quictrig.c src/agc_word.c src/agc_multiword.c tests/cm_trig_cases.c
all: build/cm-trig-native build/cm-trig-reference
build/cm-trig-native: tools/cm_trig_snapshot.c $(CM_TRIG_CORE) include/cm_quictrig.h
	gcc -O2 $(CM_TRIG_FLAGS) tools/cm_trig_snapshot.c $(CM_TRIG_CORE) -o $@
build/cm-trig-reference: tools/cm_trig_reference.c tests/cm_trig_cases.c include/cm_quictrig.h
	gcc -O2 -std=c99 -Iinclude -Itests -Ideps/virtualagc/yaAGC tools/cm_trig_reference.c tests/cm_trig_cases.c tests/cm_executive_cases.c src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	build/cm-trig-reference deps/cm-trig-fixture/MAIN.agc.bin > build/cm-trig-reference.snapshots 2> build/cm-trig-reference.trace
	build/cm-trig-native > build/cm-trig-native.snapshots
	cmp build/cm-trig-reference.snapshots build/cm-trig-native.snapshots
sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CM_TRIG_FLAGS) tools/cm_trig_snapshot.c $(CM_TRIG_CORE) -o build/cm-trig-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/cm-trig-sanitize > build/cm-trig-sanitize.snapshots
	cmp build/cm-trig-reference.snapshots build/cm-trig-sanitize.snapshots
.PHONY: all test sanitize
clock-test:
	gcc -O2 -std=c99 -Iinclude -Itests -Ideps/virtualagc/yaAGC tools/cm_clock_trig_reference.c tests/cm_trig_cases.c tests/cm_executive_cases.c src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o build/cm-clock-trig-reference
	build/cm-clock-trig-reference deps/cm-trig-fixture/MAIN.agc.bin > build/cm-clock-trig-reference.snapshots 2> build/cm-clock-trig-reference.trace
	gcc -O2 $(CM_TRIG_FLAGS) tools/cm_clock_trig_snapshot.c $(CM_TRIG_CORE) -o build/cm-clock-trig-native
	build/cm-clock-trig-native < build/cm-clock-trig-reference.snapshots > build/cm-clock-trig-native.snapshots
	python tools/cm_clock_trig_compare.py
check-host:
	gcc -O2 $(CM_TRIG_FLAGS) tests/test_cm_trig.c tests/frozen_words.c $(CM_TRIG_CORE) -o build/test-cm-trig
	build/test-cm-trig
c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DCM_TRIG_CHECK_ONLY -DCM_TRIG_SELECTED=$(if $(SELECTOR),$(SELECTOR),0) $(CM_TRIG_FLAGS) tests/test_cm_trig.c tests/frozen_words.c tools/cm_trig_diagnostic.c $(CM_TRIG_CORE) -Wl,-Map,build/cm-trig$(if $(SELECTOR),$(SELECTOR),0).map -o build/cm-trig$(if $(SELECTOR),$(SELECTOR),0).prg
clock-sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(CM_TRIG_FLAGS) tools/cm_clock_trig_snapshot.c $(CM_TRIG_CORE) -o build/cm-clock-trig-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/cm-clock-trig-sanitize < build/cm-clock-trig-reference.snapshots > build/cm-clock-trig-sanitize.snapshots
	cmp build/cm-clock-trig-native.snapshots build/cm-clock-trig-sanitize.snapshots
test-cm-trig: test clock-test check-host
