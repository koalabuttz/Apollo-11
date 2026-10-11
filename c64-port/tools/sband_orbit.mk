# SPDX-License-Identifier: MIT
CC ?= cc
SBO_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
SBO_INPUT = tests/sband_orbit_cases.c src/agc_word.c
SBO_CORE = $(SBO_INPUT) src/sband_orbit.c
build/sband-orbit-native: tools/sband_orbit_snapshot.c $(SBO_CORE)
	$(CC) -O2 $(SBO_FLAGS) $^ -o $@
build/sband-orbit-reference: tools/sband_orbit_reference.c $(SBO_INPUT)
	$(CC) -O2 $(filter-out -Wconversion,$(SBO_FLAGS)) -ffunction-sections -Wl,--gc-sections -Ideps/virtualagc/yaAGC $^ build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
all: build/sband-orbit-reference build/sband-orbit-native
test: all
	python3 tools/sband_orbit_compare.py
sanitize:
	$(CC) -O1 -g $(SBO_FLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/sband_orbit_snapshot.c $(SBO_CORE) -o build/sband-orbit-sanitize
	python3 tools/sband_orbit_compare.py sanitize
.PHONY: all test sanitize
build/sband-orbit-inputs: tools/sband_orbit_inputs.c $(SBO_INPUT)
	$(CC) -O2 $(SBO_FLAGS) $^ -o $@
freeze: build/sband-orbit-inputs
	python3 tools/freeze_sband_orbit.py
check-host:
	$(CC) -O2 $(SBO_FLAGS) tests/test_sband_orbit.c tests/sband_orbit_target.c tests/frozen_words.c $(SBO_CORE) -o build/test-sband-orbit
	./build/test-sband-orbit
MOS_CC ?= mos-c64-clang
VICE ?= x64sc
ROM_DIR ?= /workspace/toolchains/roms/C64
VICE_DATA_DIR ?= /workspace/toolchains/vice/usr/share/vice
vice-test:
	python3 tools/sband_orbit_vice_test.py --cc $(MOS_CC) --vice $(VICE) --rom-dir $(ROM_DIR) --data-dir $(VICE_DATA_DIR)
.PHONY: freeze check-host vice-test
