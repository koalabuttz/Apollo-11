# SPDX-License-Identifier: MIT
SHELL = /bin/bash
.SHELLFLAGS = -eu -o pipefail -c
IMUW_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
IMUW_INPUT = tests/imu_pulse_wake_cases.c tests/imu_pulse_cases.c
IMUW_CORE = src/imu_pulse_wake.c src/imu_pulse_service.c src/lm_basic_sleep.c src/executive.c src/cm_basic_sleep.c src/cm_executive.c src/cm_dispatch.c src/lm_waitlist_image.c src/waitlist.c src/cm_waitlist_image.c src/cm_waitlist.c src/cm_native_image.c src/agc_word.c src/agc_multiword.c $(IMUW_INPUT)
all: build/imu-pulse-wake-native build/imu-pulse-wake-reference
build/imu-pulse-wake-native: tools/imu_pulse_wake_snapshot.c $(IMUW_CORE)
	gcc -O2 $(IMUW_FLAGS) $^ -o $@
build/imu-pulse-wake-reference: tools/imu_pulse_wake_reference.c $(IMUW_INPUT)
	gcc -O2 -std=c99 -Iinclude -Itests -Ideps/virtualagc/yaAGC $^ tests/cm_executive_cases.c src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	for mission in 0 1; do for stage in 0 1; do tree=lm; if [ $$mission = 1 ]; then tree=cm; fi; name=imu-pulse-wake-$$tree-$$stage; build/imu-pulse-wake-reference deps/$$tree-imu-pulse-fixture/MAIN.agc.bin $$mission $$stage 65536 | xz -T1 -3 > build/$$name.states.xz; build/imu-pulse-wake-native $$mission $$stage 65536 | cmp - <(xz -dc build/$$name.states.xz); done; done
sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(IMUW_FLAGS) tools/imu_pulse_wake_snapshot.c $(IMUW_CORE) -o build/imu-pulse-wake-sanitize
	for mission in 0 1; do for stage in 0 1; do tree=lm; if [ $$mission = 1 ]; then tree=cm; fi; ASAN_OPTIONS=detect_leaks=0 build/imu-pulse-wake-sanitize $$mission $$stage 65536 | cmp - <(xz -dc build/imu-pulse-wake-$$tree-$$stage.states.xz); done; done
.PHONY: all test sanitize
IMUW_MISSION ?= 0
IMUW_STAGE ?= 0
IMUW_TREE = $(if $(filter 1,$(IMUW_MISSION)),cm,lm)
check-host:
	for mission in 0 1; do for stage in 0 1; do gcc -O2 $(IMUW_FLAGS) -DIMUW_MISSION=$$mission -DIMUW_STAGE=$$stage tests/test_imu_pulse_wake.c tests/frozen_words.c $(IMUW_CORE) -o build/test-imu-pulse-wake-$$mission-$$stage; build/test-imu-pulse-wake-$$mission-$$stage; done; done
c64:
	$(MOS_CC) -Wno-error=sign-conversion -Os -fno-static-stack -mlto-zp=0 -DIMU_PULSE_WAKE_CHECK_ONLY -DIMUW_MISSION=$(IMUW_MISSION) -DIMUW_STAGE=$(IMUW_STAGE) $(IMUW_FLAGS) tests/test_imu_pulse_wake.c tests/frozen_words.c tools/imu_pulse_wake_diagnostic.c $(IMUW_CORE) -Wl,-Map,build/imu-pulse-wake-$(IMUW_TREE)$(IMUW_STAGE).map -o build/imu-pulse-wake-$(IMUW_TREE)$(IMUW_STAGE).prg
vice-test: c64
	python tools/imu_pulse_wake_vice_test.py --mission $(IMUW_MISSION) --stage $(IMUW_STAGE) --vice $(VICE) --rom-dir $(C64_ROM_DIR) --data-dir $(VICE_DATA_DIR) | tee build/imu-pulse-wake-$(IMUW_TREE)$(IMUW_STAGE)-validation.log
.PHONY: check-host c64 vice-test
