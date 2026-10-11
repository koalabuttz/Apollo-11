# SPDX-License-Identifier: MIT
CC ?= cc
SBS_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
SBS_INPUT = tests/sband_shift_cases.c src/agc_word.c
SBS_CORE = $(SBS_INPUT) src/sband_shift.c src/agc_multiword.c src/guidance_shift.c
build/sband-shift-native: tools/sband_shift_snapshot.c $(SBS_CORE)
	$(CC) -O2 $(SBS_FLAGS) $^ -o $@
build/sband-shift-reference: tools/sband_shift_reference.c $(SBS_INPUT)
	$(CC) -O2 $(filter-out -Wconversion,$(SBS_FLAGS)) -ffunction-sections -Wl,--gc-sections -Ideps/virtualagc/yaAGC $^ build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
all: build/sband-shift-native build/sband-shift-reference
test: all
	python3 tools/sband_shift_compare.py
sanitize:
	$(CC) -O1 -g $(SBS_FLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/sband_shift_snapshot.c $(SBS_CORE) -o build/sband-shift-sanitize
	python3 tools/sband_shift_compare.py sanitize
.PHONY: all test sanitize
build/sband-shift-inputs: tools/sband_shift_inputs.c $(SBS_INPUT)
	$(CC) -O2 $(SBS_FLAGS) $^ -o $@
freeze: build/sband-shift-inputs
	python3 tools/freeze_sband_shift.py
check-host:
	$(CC) -O2 $(SBS_FLAGS) tests/test_sband_shift.c tests/sband_shift_target.c tests/frozen_words.c $(SBS_CORE) -o build/test-sband-shift
	./build/test-sband-shift
MOS_CC ?= mos-c64-clang
VICE ?= x64sc
ROM_DIR ?= /workspace/toolchains/roms/C64
VICE_DATA_DIR ?= /workspace/toolchains/vice/usr/share/vice
vice-test:
	python3 tools/sband_shift_vice_test.py --cc $(MOS_CC) --vice $(VICE) --rom-dir $(ROM_DIR) --data-dir $(VICE_DATA_DIR)
.PHONY: freeze check-host vice-test
