# SPDX-License-Identifier: MIT
CC ?= cc
MOS_CC ?= mos-c64-clang
VICE ?= x64sc
ROM_DIR ?= /workspace/toolchains/roms/C64
VICE_DATA_DIR ?= /workspace/toolchains/vice/usr/share/vice
SV_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
SV_INPUT = tests/sband_vector_cases.c src/agc_word.c
SV_CORE = $(SV_INPUT) src/agc_multiword.c src/sband_vector.c
all: build/sband-vector-native build/sband-vector-reference
build/sband-vector-native: tools/sband_vector_snapshot.c $(SV_CORE)
	$(CC) -O2 $(SV_FLAGS) $^ -o $@
build/sband-vector-reference: tools/sband_vector_reference.c $(SV_INPUT)
	$(CC) -O2 $(filter-out -Wconversion,$(SV_FLAGS)) -ffunction-sections -Wl,--gc-sections -Ideps/virtualagc/yaAGC $^ build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	python3 tools/sband_vector_compare.py
sanitize:
	$(CC) -O1 -g $(SV_FLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer tools/sband_vector_snapshot.c $(SV_CORE) -o build/sband-vector-sanitize
	python3 tools/sband_vector_compare.py sanitize
.PHONY: all test sanitize
build/sband-vector-inputs: tools/sband_vector_inputs.c $(SV_INPUT)
	$(CC) -O2 $(SV_FLAGS) $^ -o $@
freeze: build/sband-vector-inputs
	python3 tools/freeze_sband_vector.py
check-host:
	$(CC) -O2 $(SV_FLAGS) tests/test_sband_vector.c tests/sband_vector_target.c tests/frozen_words.c $(SV_CORE) -o build/test-sband-vector
	./build/test-sband-vector
.PHONY: freeze check-host

vice-test:
	python3 tools/sband_vector_vice_test.py --cc $(MOS_CC) --vice $(VICE) --rom-dir $(ROM_DIR) --data-dir $(VICE_DATA_DIR)
.PHONY: vice-test
