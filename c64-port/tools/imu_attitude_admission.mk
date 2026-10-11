# SPDX-License-Identifier: MIT
SHELL = /bin/bash
.SHELLFLAGS = -eu -o pipefail -c
IMUA_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
IMUA_INPUT = tests/imu_attitude_admission_cases.c tests/imu_mode_control_cases.c tests/imu_coarse_cases.c tests/imu_pulse_cases.c
IMUA_CORE = src/imu_attitude_admission.c src/imu_mode_control.c src/imu_coarse.c src/agc_word.c src/agc_multiword.c $(IMUA_INPUT)
all: build/imu-attitude-admission-native build/imu-attitude-admission-reference
build/imu-attitude-admission-native: tools/imu_attitude_admission_snapshot.c $(IMUA_CORE)
	gcc -O2 $(IMUA_FLAGS) $^ -o $@
build/imu-attitude-admission-reference: tools/imu_attitude_admission_reference.c $(IMUA_INPUT)
	gcc -O2 -std=c99 -Iinclude -Itests -Ideps/virtualagc/yaAGC $^ tests/cm_executive_cases.c src/agc_word.c build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	python3 tools/imu_attitude_admission_host.py --generate
sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(IMUA_FLAGS) tools/imu_attitude_admission_snapshot.c $(IMUA_CORE) -o build/imu-attitude-admission-native-san
	python3 tools/imu_attitude_admission_host.py --sanitize
.PHONY: all test sanitize
IMUA_MISSION ?= 0
IMUA_ENTRY ?= 0
IMUA_TREE = $(if $(filter 1,$(IMUA_MISSION)),cm,lm)
check-host:
	for mission in 0 1; do for entry in $$(seq 0 4); do gcc -O2 $(IMUA_FLAGS) -DIMUA_MISSION=$$mission -DIMUA_ENTRY=$$entry tests/test_imu_attitude_admission.c tests/frozen_words.c $(IMUA_CORE) -o build/test-imu-attitude-admission-$$mission-$$entry; build/test-imu-attitude-admission-$$mission-$$entry; done; done
c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DIMU_ATTITUDE_ADMISSION_CHECK_ONLY -DIMUA_MISSION=$(IMUA_MISSION) -DIMUA_ENTRY=$(IMUA_ENTRY) $(IMUA_FLAGS) tests/test_imu_attitude_admission.c tests/frozen_words.c tools/imu_attitude_admission_diagnostic.c $(IMUA_CORE) -Wl,-Map,build/imu-attitude-admission-$(IMUA_TREE)$(IMUA_ENTRY).map -o build/imu-attitude-admission-$(IMUA_TREE)$(IMUA_ENTRY).prg
vice-test: c64
	python tools/imu_attitude_admission_vice_test.py --mission $(IMUA_MISSION) --entry $(IMUA_ENTRY) --vice $(VICE) --rom-dir $(C64_ROM_DIR) --data-dir $(VICE_DATA_DIR) | tee build/imu-attitude-admission-$(IMUA_TREE)$(IMUA_ENTRY)-validation.log
.PHONY: check-host c64 vice-test
