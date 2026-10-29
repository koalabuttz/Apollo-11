# SPDX-License-Identifier: MIT
SHELL = /bin/bash
.SHELLFLAGS = -eu -o pipefail -c
IMUP_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
IMUP_CORE = src/imu_pulse.c src/imu_pulse_service.c src/lm_waitlist_image.c src/waitlist.c src/cm_waitlist_image.c src/cm_waitlist.c src/cm_native_image.c src/agc_multiword.c src/agc_word.c tests/imu_pulse_cases.c
all: build/imu-pulse-native build/imu-pulse-reference
build/imu-pulse-native: tools/imu_pulse_snapshot.c $(IMUP_CORE)
	gcc -O2 $(IMUP_FLAGS) tools/imu_pulse_snapshot.c $(IMUP_CORE) -o $@
build/imu-pulse-reference: tools/imu_pulse_reference.c tests/imu_pulse_cases.c
	gcc -O2 -std=c99 -Iinclude -Itests -Ideps/virtualagc/yaAGC tools/imu_pulse_reference.c tests/imu_pulse_cases.c tests/cm_executive_cases.c src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	for mission in 0 1; do for stage in 0 1 2; do tree=lm; if [ $$mission = 1 ]; then tree=cm; fi; name=imu-pulse-$$tree-$$stage; build/imu-pulse-reference deps/$$tree-imu-pulse-fixture/MAIN.agc.bin $$mission 65536 $$stage | xz -T1 -3 > build/$$name.states.xz; build/imu-pulse-native $$mission 65536 $$stage | cmp - <(xz -dc build/$$name.states.xz); done; done
sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(IMUP_FLAGS) tools/imu_pulse_snapshot.c $(IMUP_CORE) -o build/imu-pulse-sanitize
	for mission in 0 1; do for stage in 0 1 2; do tree=lm; if [ $$mission = 1 ]; then tree=cm; fi; ASAN_OPTIONS=detect_leaks=0 build/imu-pulse-sanitize $$mission 65536 $$stage | cmp - <(xz -dc build/imu-pulse-$$tree-$$stage.states.xz); done; done
.PHONY: all test sanitize
IMUP_MISSION ?= 0
IMUP_STAGE ?= 0
IMUP_TREE = $(if $(filter 1,$(IMUP_MISSION)),cm,lm)
check-host:
	for mission in 0 1; do for stage in 0 1 2; do gcc -O2 $(IMUP_FLAGS) -DIMUP_MISSION=$$mission -DIMUP_STAGE=$$stage tests/test_imu_pulse.c tests/frozen_words.c $(IMUP_CORE) -o build/test-imu-pulse-$$mission-$$stage; build/test-imu-pulse-$$mission-$$stage; done; done
c64:
	$(MOS_CC) -Wno-error=sign-conversion -Os -fno-static-stack -mlto-zp=0 -DIMU_PULSE_CHECK_ONLY -DIMUP_MISSION=$(IMUP_MISSION) -DIMUP_STAGE=$(IMUP_STAGE) $(IMUP_FLAGS) tests/test_imu_pulse.c tests/frozen_words.c tools/imu_pulse_diagnostic.c $(IMUP_CORE) -Wl,-Map,build/imu-pulse-$(IMUP_TREE)$(IMUP_STAGE).map -o build/imu-pulse-$(IMUP_TREE)$(IMUP_STAGE).prg
vice-test: c64
	python tools/imu_pulse_vice_test.py --mission $(IMUP_MISSION) --stage $(IMUP_STAGE) --vice $(VICE) --rom-dir $(C64_ROM_DIR) --data-dir $(VICE_DATA_DIR) | tee build/imu-pulse-$(IMUP_TREE)$(IMUP_STAGE)-validation.log
.PHONY: check-host c64 vice-test
