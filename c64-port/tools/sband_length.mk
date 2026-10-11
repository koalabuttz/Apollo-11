# SPDX-License-Identifier: MIT
CC ?= cc
SBO_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
SBO_INPUT = tests/sband_length_cases.c src/agc_word.c
SBO_CORE = $(SBO_INPUT) src/sband_length.c src/guidance_length_phased.c src/guidance_norm.c src/agc_multiword.c
build/sband-length-native: tools/sband_length_snapshot.c $(SBO_CORE)
	$(CC) -O2 $(SBO_FLAGS) $^ -o $@
build/sband-length-reference: tools/sband_length_reference.c $(SBO_INPUT)
	$(CC) -O2 $(filter-out -Wconversion,$(SBO_FLAGS)) -ffunction-sections -Wl,--gc-sections -Ideps/virtualagc/yaAGC $^ build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
all: build/sband-length-reference build/sband-length-native
test: all
	python3 tools/sband_length_compare.py
sanitize:
	$(CC) -O1 -g $(SBO_FLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/sband_length_snapshot.c $(SBO_CORE) -o build/sband-length-sanitize
	python3 tools/sband_length_compare.py sanitize
.PHONY: all test sanitize
