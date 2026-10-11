# SPDX-License-Identifier: MIT
SHELL = /bin/bash
.SHELLFLAGS = -eu -o pipefail -c
IMUE_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
IMUE_INPUT = tests/imu_pulse_end_cases.c tests/imu_pulse_cases.c
IMUE_CORE = src/imu_pulse_end.c src/lm_basic_wake.c src/executive.c src/cm_basic_wake.c src/cm_executive.c src/cm_dispatch.c src/cm_native_image.c src/agc_word.c src/agc_multiword.c $(IMUE_INPUT)
all: build/imu-pulse-end-native build/imu-pulse-end-reference
build/imu-pulse-end-native: tools/imu_pulse_end_snapshot.c $(IMUE_CORE)
	gcc -O2 $(IMUE_FLAGS) $^ -o $@
build/imu-pulse-end-reference: tools/imu_pulse_end_reference.c $(IMUE_INPUT)
	gcc -O2 -std=c99 -Iinclude -Itests -Ideps/virtualagc/yaAGC $^ tests/cm_executive_cases.c src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	for mission in 0 1; do for entry in 0 1; do tree=lm; if [ $$mission = 1 ]; then tree=cm; fi; name=imu-pulse-end-$$tree-$$entry; build/imu-pulse-end-reference deps/$$tree-imu-pulse-fixture/MAIN.agc.bin $$mission $$entry 65536 | xz -T1 -3 > build/$$name.states.xz; build/imu-pulse-end-native $$mission $$entry 65536 | cmp - <(xz -dc build/$$name.states.xz); done; done
sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(IMUE_FLAGS) tools/imu_pulse_end_snapshot.c $(IMUE_CORE) -o build/imu-pulse-end-sanitize
	for mission in 0 1; do for entry in 0 1; do tree=lm; if [ $$mission = 1 ]; then tree=cm; fi; ASAN_OPTIONS=detect_leaks=0 build/imu-pulse-end-sanitize $$mission $$entry 65536 | cmp - <(xz -dc build/imu-pulse-end-$$tree-$$entry.states.xz); done; done
.PHONY: all test sanitize
IMUE_MISSION ?= 0
IMUE_ENTRY ?= 0
IMUE_TREE = $(if $(filter 1,$(IMUE_MISSION)),cm,lm)
check-host:
	for mission in 0 1; do for entry in 0 1; do gcc -O2 $(IMUE_FLAGS) -DIMUE_MISSION=$$mission -DIMUE_ENTRY=$$entry tests/test_imu_pulse_end.c tests/frozen_words.c $(IMUE_CORE) -o build/test-imu-pulse-end-$$mission-$$entry; build/test-imu-pulse-end-$$mission-$$entry; done; done
c64:
	$(MOS_CC) -Wno-error=sign-conversion -Os -fno-static-stack -mlto-zp=0 -DIMU_PULSE_END_CHECK_ONLY -DIMUE_MISSION=$(IMUE_MISSION) -DIMUE_ENTRY=$(IMUE_ENTRY) $(IMUE_FLAGS) tests/test_imu_pulse_end.c tests/frozen_words.c tools/imu_pulse_end_diagnostic.c $(IMUE_CORE) -Wl,-Map,build/imu-pulse-end-$(IMUE_TREE)$(IMUE_ENTRY).map -o build/imu-pulse-end-$(IMUE_TREE)$(IMUE_ENTRY).prg
vice-test: c64
	python tools/imu_pulse_end_vice_test.py --mission $(IMUE_MISSION) --entry $(IMUE_ENTRY) --vice $(VICE) --rom-dir $(C64_ROM_DIR) --data-dir $(VICE_DATA_DIR) | tee build/imu-pulse-end-$(IMUE_TREE)$(IMUE_ENTRY)-validation.log
.PHONY: check-host c64 vice-test
