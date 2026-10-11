# SPDX-License-Identifier: MIT
SHELL = /bin/bash
.SHELLFLAGS = -eu -o pipefail -c
IMUM_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
IMUM_INPUT = tests/imu_mode_control_cases.c tests/imu_coarse_cases.c tests/imu_pulse_cases.c
IMUM_CORE = src/imu_mode_control.c src/imu_coarse.c src/agc_word.c src/agc_multiword.c $(IMUM_INPUT)
all: build/imu-mode-control-native build/imu-mode-control-reference
build/imu-mode-control-native: tools/imu_mode_control_snapshot.c $(IMUM_CORE)
	gcc -O2 $(IMUM_FLAGS) $^ -o $@
build/imu-mode-control-reference: tools/imu_mode_control_reference.c $(IMUM_INPUT)
	gcc -O2 -std=c99 -Iinclude -Itests -Ideps/virtualagc/yaAGC $^ tests/cm_executive_cases.c src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	python3 tools/imu_mode_control_host.py --generate
sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(IMUM_FLAGS) tools/imu_mode_control_snapshot.c $(IMUM_CORE) -o build/imu-mode-control-native-san
	python3 tools/imu_mode_control_host.py --sanitize
.PHONY: all test sanitize
IMUM_MISSION ?= 0
IMUM_ENTRY ?= 0
IMUM_TREE = $(if $(filter 1,$(IMUM_MISSION)),cm,lm)
check-host:
	for mission in 0 1; do for entry in $$(seq 0 19); do gcc -O2 $(IMUM_FLAGS) -DIMUM_MISSION=$$mission -DIMUM_ENTRY=$$entry tests/test_imu_mode_control.c tests/frozen_words.c $(IMUM_CORE) -o build/test-imu-mode-control-$$mission-$$entry; build/test-imu-mode-control-$$mission-$$entry; done; done
c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DIMU_MODE_CONTROL_CHECK_ONLY -DIMUM_MISSION=$(IMUM_MISSION) -DIMUM_ENTRY=$(IMUM_ENTRY) $(IMUM_FLAGS) tests/test_imu_mode_control.c tests/frozen_words.c tools/imu_mode_control_diagnostic.c $(IMUM_CORE) -Wl,-Map,build/imu-mode-control-$(IMUM_TREE)$(IMUM_ENTRY).map -o build/imu-mode-control-$(IMUM_TREE)$(IMUM_ENTRY).prg
vice-test: c64
	python tools/imu_mode_control_vice_test.py --mission $(IMUM_MISSION) --entry $(IMUM_ENTRY) --vice $(VICE) --rom-dir $(C64_ROM_DIR) --data-dir $(VICE_DATA_DIR) | tee build/imu-mode-control-$(IMUM_TREE)$(IMUM_ENTRY)-validation.log
.PHONY: check-host c64 vice-test
