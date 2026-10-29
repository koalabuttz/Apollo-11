# SPDX-License-Identifier: MIT
SHELL = /bin/bash
.SHELLFLAGS = -eu -o pipefail -c
IMUS_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
IMUS_INPUT = tests/imu_standby_cases.c tests/imu_mode_control_cases.c tests/imu_coarse_cases.c tests/imu_pulse_cases.c
IMUS_CORE = src/imu_standby.c src/imu_mode_control.c src/imu_coarse.c src/agc_word.c src/agc_multiword.c $(IMUS_INPUT)
all: build/imu-standby-native build/imu-standby-reference
build/imu-standby-native: tools/imu_standby_snapshot.c $(IMUS_CORE)
	gcc -O2 $(IMUS_FLAGS) $^ -o $@
build/imu-standby-reference: tools/imu_standby_reference.c $(IMUS_INPUT)
	gcc -O2 -std=c99 -Iinclude -Itests -Ideps/virtualagc/yaAGC $^ tests/cm_executive_cases.c src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	python3 tools/imu_standby_host.py --generate
sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(IMUS_FLAGS) tools/imu_standby_snapshot.c $(IMUS_CORE) -o build/imu-standby-native-san
	python3 tools/imu_standby_host.py --sanitize
.PHONY: all test sanitize
IMUS_MISSION ?= 0
IMUS_ENTRY ?= 0
IMUS_TREE = $(if $(filter 1,$(IMUS_MISSION)),cm,lm)
check-host:
	for mission in 0 1; do for entry in $$(seq 0 20); do gcc -O2 $(IMUS_FLAGS) -DIMUS_MISSION=$$mission -DIMUS_ENTRY=$$entry tests/test_imu_standby.c tests/frozen_words.c $(IMUS_CORE) -o build/test-imu-standby-$$mission-$$entry; build/test-imu-standby-$$mission-$$entry; done; done
c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DIMU_STANDBY_CHECK_ONLY -DIMUS_MISSION=$(IMUS_MISSION) -DIMUS_ENTRY=$(IMUS_ENTRY) $(IMUS_FLAGS) tests/test_imu_standby.c tests/frozen_words.c tools/imu_standby_diagnostic.c $(IMUS_CORE) -Wl,-Map,build/imu-standby-$(IMUS_TREE)$(IMUS_ENTRY).map -o build/imu-standby-$(IMUS_TREE)$(IMUS_ENTRY).prg
vice-test: c64
	python tools/imu_standby_vice_test.py --mission $(IMUS_MISSION) --entry $(IMUS_ENTRY) --vice $(VICE) --rom-dir $(C64_ROM_DIR) --data-dir $(VICE_DATA_DIR) | tee build/imu-standby-$(IMUS_TREE)$(IMUS_ENTRY)-validation.log
.PHONY: check-host c64 vice-test
