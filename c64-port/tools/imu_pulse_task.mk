# SPDX-License-Identifier: MIT
SHELL = /bin/bash
.SHELLFLAGS = -eu -o pipefail -c
IMUG_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
IMUG_INPUT = tests/imu_pulse_task_cases.c tests/imu_pulse_cases.c
IMUG_CORE = src/imu_pulse_task.c src/imu_pulse_task_register.c src/lm_waitlist_image.c src/waitlist.c src/cm_waitlist_image.c src/cm_waitlist.c src/cm_native_image.c src/agc_word.c src/agc_multiword.c $(IMUG_INPUT)
all: build/imu-pulse-task-native build/imu-pulse-task-reference
build/imu-pulse-task-native: tools/imu_pulse_task_snapshot.c $(IMUG_CORE)
	gcc -O2 $(IMUG_FLAGS) $^ -o $@
build/imu-pulse-task-reference: tools/imu_pulse_task_reference.c $(IMUG_INPUT)
	gcc -O2 -std=c99 -Iinclude -Itests -Ideps/virtualagc/yaAGC $^ tests/cm_executive_cases.c src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	for mission in 0 1; do for augment in 0 1; do for stage in 0 1; do tree=lm; if [ $$mission = 1 ]; then tree=cm; fi; name=imu-pulse-task-$$tree-$$augment-$$stage; build/imu-pulse-task-reference deps/$$tree-imu-pulse-fixture/MAIN.agc.bin $$mission $$augment 65536 $$stage | xz -T1 -3 > build/$$name.states.xz; build/imu-pulse-task-native $$mission $$augment 65536 $$stage | cmp - <(xz -dc build/$$name.states.xz); done; done; done
sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(IMUG_FLAGS) tools/imu_pulse_task_snapshot.c $(IMUG_CORE) -o build/imu-pulse-task-sanitize
	for mission in 0 1; do for augment in 0 1; do for stage in 0 1; do tree=lm; if [ $$mission = 1 ]; then tree=cm; fi; ASAN_OPTIONS=detect_leaks=0 build/imu-pulse-task-sanitize $$mission $$augment 65536 $$stage | cmp - <(xz -dc build/imu-pulse-task-$$tree-$$augment-$$stage.states.xz); done; done; done
.PHONY: all test sanitize
IMUG_MISSION ?= 0
IMUG_AUGMENT ?= 0
IMUG_STAGE ?= 0
IMUG_TREE = $(if $(filter 1,$(IMUG_MISSION)),cm,lm)
check-host:
	for mission in 0 1; do for augment in 0 1; do for stage in 0 1; do gcc -O2 $(IMUG_FLAGS) -DIMUG_MISSION=$$mission -DIMUG_AUGMENT=$$augment -DIMUG_STAGE=$$stage tests/test_imu_pulse_task.c tests/frozen_words.c $(IMUG_CORE) -o build/test-imu-pulse-task-$$mission-$$augment-$$stage; build/test-imu-pulse-task-$$mission-$$augment-$$stage; done; done; done
c64:
	$(MOS_CC) -Wno-error=sign-conversion -Os -fno-static-stack -mlto-zp=0 -DIMU_PULSE_TASK_CHECK_ONLY -DIMUG_MISSION=$(IMUG_MISSION) -DIMUG_AUGMENT=$(IMUG_AUGMENT) -DIMUG_STAGE=$(IMUG_STAGE) $(IMUG_FLAGS) tests/test_imu_pulse_task.c tests/frozen_words.c tools/imu_pulse_task_diagnostic.c $(IMUG_CORE) -Wl,-Map,build/imu-pulse-task-$(IMUG_TREE)$(IMUG_AUGMENT)$(IMUG_STAGE).map -o build/imu-pulse-task-$(IMUG_TREE)$(IMUG_AUGMENT)$(IMUG_STAGE).prg
vice-test: c64
	python tools/imu_pulse_task_vice_test.py --mission $(IMUG_MISSION) --augment $(IMUG_AUGMENT) --stage $(IMUG_STAGE) --vice $(VICE) --rom-dir $(C64_ROM_DIR) --data-dir $(VICE_DATA_DIR) | tee build/imu-pulse-task-$(IMUG_TREE)$(IMUG_AUGMENT)$(IMUG_STAGE)-validation.log
.PHONY: check-host c64 vice-test
