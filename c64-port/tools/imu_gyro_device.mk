# SPDX-License-Identifier: MIT
SHELL = /bin/bash
.SHELLFLAGS = -eu -o pipefail -c
IMUD_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
IMUD_INPUT = tests/imu_gyro_device_cases.c tests/imu_pulse_cases.c
IMUD_CORE = src/imu_gyro_device.c src/agc_word.c src/agc_multiword.c $(IMUD_INPUT)
all: build/imu-gyro-device-native build/imu-gyro-device-reference-0 build/imu-gyro-device-reference-1
build/imu-gyro-device-native: tools/imu_gyro_device_snapshot.c $(IMUD_CORE)
	gcc -O2 $(IMUD_FLAGS) $^ -o $@
build/imu-gyro-device-reference-0: tools/imu_gyro_device_reference.c $(IMUD_INPUT)
	gcc -O2 -std=c99 -Dunix -Iinclude -Itests -Ideps/virtualagc/yaAGC $^ tests/cm_executive_cases.c src/agc_word.c deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
build/imu-gyro-device-reference-1: tools/imu_gyro_device_reference.c $(IMUD_INPUT)
	gcc -O2 -std=c99 -Dunix -DGYRO_TIMING_SIMULATED -Iinclude -Itests -Ideps/virtualagc/yaAGC $^ tests/cm_executive_cases.c src/agc_word.c deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	for mission in 0 1; do for mode in 0 1; do tree=lm; if [ $$mission = 1 ]; then tree=cm; fi; name=imu-gyro-device-$$tree-$$mode; build/imu-gyro-device-reference-$$mode deps/$$tree-imu-pulse-fixture/MAIN.agc.bin $$mission $$mode 65536 | xz -T1 -3 > build/$$name.states.xz; build/imu-gyro-device-native $$mission $$mode 65536 | cmp - <(xz -dc build/$$name.states.xz); done; done
sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(IMUD_FLAGS) tools/imu_gyro_device_snapshot.c $(IMUD_CORE) -o build/imu-gyro-device-sanitize
	for mission in 0 1; do for mode in 0 1; do tree=lm; if [ $$mission = 1 ]; then tree=cm; fi; ASAN_OPTIONS=detect_leaks=0 build/imu-gyro-device-sanitize $$mission $$mode 65536 | cmp - <(xz -dc build/imu-gyro-device-$$tree-$$mode.states.xz); done; done
.PHONY: all test sanitize
IMUD_MISSION ?= 0
IMUD_MODE ?= 0
IMUD_TREE = $(if $(filter 1,$(IMUD_MISSION)),cm,lm)
check-host:
	for mission in 0 1; do for mode in 0 1; do gcc -O2 $(IMUD_FLAGS) -DIMUD_MISSION=$$mission -DIMUD_MODE=$$mode tests/test_imu_gyro_device.c tests/frozen_words.c $(IMUD_CORE) -o build/test-imu-gyro-device-$$mission-$$mode; build/test-imu-gyro-device-$$mission-$$mode; done; done
c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DIMU_GYRO_DEVICE_CHECK_ONLY -DIMUD_MISSION=$(IMUD_MISSION) -DIMUD_MODE=$(IMUD_MODE) $(IMUD_FLAGS) tests/test_imu_gyro_device.c tests/frozen_words.c tools/imu_gyro_device_diagnostic.c $(IMUD_CORE) -Wl,-Map,build/imu-gyro-device-$(IMUD_TREE)$(IMUD_MODE).map -o build/imu-gyro-device-$(IMUD_TREE)$(IMUD_MODE).prg
vice-test: c64
	python tools/imu_gyro_device_vice_test.py --mission $(IMUD_MISSION) --mode $(IMUD_MODE) --vice $(VICE) --rom-dir $(C64_ROM_DIR) --data-dir $(VICE_DATA_DIR) | tee build/imu-gyro-device-$(IMUD_TREE)$(IMUD_MODE)-validation.log
.PHONY: check-host c64 vice-test
