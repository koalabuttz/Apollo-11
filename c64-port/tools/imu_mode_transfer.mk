# SPDX-License-Identifier: MIT
SHELL = /bin/bash
.SHELLFLAGS = -eu -o pipefail -c
IMUT_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
IMUT_INPUT = tests/imu_mode_transfer_cases.c tests/imu_mode_control_cases.c tests/imu_coarse_cases.c tests/imu_pulse_cases.c
IMUT_CORE = src/imu_mode_transfer.c src/imu_mode_control.c src/imu_coarse.c src/agc_word.c src/agc_multiword.c $(IMUT_INPUT)
all: build/imu-mode-transfer-native build/imu-mode-transfer-reference
build/imu-mode-transfer-native: tools/imu_mode_transfer_snapshot.c $(IMUT_CORE)
	gcc -O2 $(IMUT_FLAGS) $^ -o $@
build/imu-mode-transfer-reference: tools/imu_mode_transfer_reference.c $(IMUT_INPUT)
	gcc -O2 -std=c99 -Iinclude -Itests -Ideps/virtualagc/yaAGC $^ tests/cm_executive_cases.c src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	python3 tools/imu_mode_transfer_host.py --generate
sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(IMUT_FLAGS) tools/imu_mode_transfer_snapshot.c $(IMUT_CORE) -o build/imu-mode-transfer-native-san
	python3 tools/imu_mode_transfer_host.py --sanitize
.PHONY: all test sanitize
IMUT_MISSION ?= 0
IMUT_ENTRY ?= 0
IMUT_TREE = $(if $(filter 1,$(IMUT_MISSION)),cm,lm)
check-host:
	for mission in 0 1; do for entry in $$(seq 0 10); do gcc -O2 $(IMUT_FLAGS) -DIMUT_MISSION=$$mission -DIMUT_ENTRY=$$entry tests/test_imu_mode_transfer.c tests/frozen_words.c $(IMUT_CORE) -o build/test-imu-mode-transfer-$$mission-$$entry; build/test-imu-mode-transfer-$$mission-$$entry; done; done
c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DIMU_MODE_TRANSFER_CHECK_ONLY -DIMUT_MISSION=$(IMUT_MISSION) -DIMUT_ENTRY=$(IMUT_ENTRY) $(IMUT_FLAGS) tests/test_imu_mode_transfer.c tests/frozen_words.c tools/imu_mode_transfer_diagnostic.c $(IMUT_CORE) -Wl,-Map,build/imu-mode-transfer-$(IMUT_TREE)$(IMUT_ENTRY).map -o build/imu-mode-transfer-$(IMUT_TREE)$(IMUT_ENTRY).prg
vice-test: c64
	python tools/imu_mode_transfer_vice_test.py --mission $(IMUT_MISSION) --entry $(IMUT_ENTRY) --vice $(VICE) --rom-dir $(C64_ROM_DIR) --data-dir $(VICE_DATA_DIR) | tee build/imu-mode-transfer-$(IMUT_TREE)$(IMUT_ENTRY)-validation.log
.PHONY: check-host c64 vice-test
