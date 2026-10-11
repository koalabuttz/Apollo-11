# SPDX-License-Identifier: MIT
CC ?= cc
SBS_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
SBS_INPUT = tests/sband_scalar_cases.c src/agc_word.c
SBS_CORE = $(SBS_INPUT) src/sband_scalar.c src/agc_multiword.c
build/sband-scalar-native: tools/sband_scalar_snapshot.c $(SBS_CORE)
	$(CC) -O2 $(SBS_FLAGS) $^ -o $@
build/sband-scalar-reference: tools/sband_scalar_reference.c $(SBS_INPUT)
	$(CC) -O2 $(filter-out -Wconversion,$(SBS_FLAGS)) -ffunction-sections -Wl,--gc-sections -Ideps/virtualagc/yaAGC $^ build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
all: build/sband-scalar-native build/sband-scalar-reference
test: all
	python3 tools/sband_scalar_compare.py
sanitize:
	$(CC) -O1 -g $(SBS_FLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/sband_scalar_snapshot.c $(SBS_CORE) -o build/sband-scalar-sanitize
	python3 tools/sband_scalar_compare.py sanitize
.PHONY: all test sanitize
