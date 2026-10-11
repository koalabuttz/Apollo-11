# SPDX-License-Identifier: MIT
CC ?= cc
SBC_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
SBC_INPUT = tests/sband_control_cases.c src/agc_word.c
SBC_CORE = $(SBC_INPUT) src/sband_control.c src/lm_interbank.c src/cm_interbank_complete.c src/service_routines.c
all: build/sband-control-native build/sband-control-reference
build/sband-control-native: tools/sband_control_snapshot.c $(SBC_CORE)
	$(CC) -O2 $(SBC_FLAGS) $^ -o $@
build/sband-control-reference: tools/sband_control_reference.c $(SBC_INPUT)
	$(CC) -O2 $(filter-out -Wconversion,$(SBC_FLAGS)) -ffunction-sections -Wl,--gc-sections -Ideps/virtualagc/yaAGC $^ build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
.PHONY: all
test: all
	python3 tools/sband_control_compare.py
sanitize:
	$(CC) -O1 -g $(SBC_FLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer tools/sband_control_snapshot.c $(SBC_CORE) -o build/sband-control-sanitize
	python3 tools/sband_control_compare.py sanitize
.PHONY: test sanitize
build/sband-control-inputs: tools/sband_control_inputs.c $(SBC_INPUT)
	$(CC) -O2 $(SBC_FLAGS) $^ -o $@
freeze: build/sband-control-inputs
	python3 tools/freeze_sband_control.py
check-host:
	$(CC) -O2 $(SBC_FLAGS) tests/test_sband_control.c tests/sband_control_target.c tests/frozen_words.c $(SBC_CORE) -o build/test-sband-control
	./build/test-sband-control
.PHONY: freeze check-host
MOS_CC ?= mos-c64-clang
VICE ?= x64sc
ROM_DIR ?= /workspace/toolchains/roms/C64
VICE_DATA_DIR ?= /workspace/toolchains/vice/usr/share/vice
source-audit:
	python3 tools/sband_control_source.py
vice-test:
	python3 tools/sband_control_vice_test.py --cc $(MOS_CC) --vice $(VICE) --rom-dir $(ROM_DIR) --data-dir $(VICE_DATA_DIR)
.PHONY: source-audit vice-test
