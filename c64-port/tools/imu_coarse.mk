# SPDX-License-Identifier: MIT
SHELL = /bin/bash
.SHELLFLAGS = -eu -o pipefail -c
IMUC_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
IMUC_INPUT = tests/imu_coarse_cases.c tests/imu_pulse_cases.c
IMUC_CORE = src/imu_coarse.c src/agc_word.c src/agc_multiword.c $(IMUC_INPUT)
all: build/imu-coarse-native build/imu-coarse-reference
build/imu-coarse-native: tools/imu_coarse_snapshot.c $(IMUC_CORE)
	gcc -O2 $(IMUC_FLAGS) $^ -o $@
build/imu-coarse-reference: tools/imu_coarse_reference.c $(IMUC_INPUT)
	gcc -O2 -std=c99 -Iinclude -Itests -Ideps/virtualagc/yaAGC $^ tests/cm_executive_cases.c src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	python3 tools/imu_coarse_host.py --generate
sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(IMUC_FLAGS) tools/imu_coarse_snapshot.c $(IMUC_CORE) -o build/imu-coarse-native-san
	python3 tools/imu_coarse_host.py --sanitize
.PHONY: all test sanitize
IMUC_MISSION ?= 0
IMUC_ENTRY ?= 0
IMUC_TREE = $(if $(filter 1,$(IMUC_MISSION)),cm,lm)
check-host:
	for mission in 0 1; do for entry in 0 1 2 3; do gcc -O2 $(IMUC_FLAGS) -DIMUC_MISSION=$$mission -DIMUC_ENTRY=$$entry tests/test_imu_coarse.c tests/frozen_words.c $(IMUC_CORE) -o build/test-imu-coarse-$$mission-$$entry; build/test-imu-coarse-$$mission-$$entry; done; done
c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DIMU_COARSE_CHECK_ONLY -DIMUC_MISSION=$(IMUC_MISSION) -DIMUC_ENTRY=$(IMUC_ENTRY) $(IMUC_FLAGS) tests/test_imu_coarse.c tests/frozen_words.c tools/imu_coarse_diagnostic.c $(IMUC_CORE) -Wl,-Map,build/imu-coarse-$(IMUC_TREE)$(IMUC_ENTRY).map -o build/imu-coarse-$(IMUC_TREE)$(IMUC_ENTRY).prg
vice-test: c64
	python tools/imu_coarse_vice_test.py --mission $(IMUC_MISSION) --entry $(IMUC_ENTRY) --vice $(VICE) --rom-dir $(C64_ROM_DIR) --data-dir $(VICE_DATA_DIR) | tee build/imu-coarse-$(IMUC_TREE)$(IMUC_ENTRY)-validation.log
.PHONY: check-host c64 vice-test
