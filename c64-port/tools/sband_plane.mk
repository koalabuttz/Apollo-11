# SPDX-License-Identifier: MIT
CC ?= cc
SBB_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
SBB_INPUT = tests/sband_plane_cases.c src/agc_word.c
SBB_CORE = $(SBB_INPUT) src/sband_plane.c src/sband_vector.c src/sband_shift.c src/guidance_shift.c src/rtb_unit.c src/rtb_vector.c src/guidance_unit_phased.c src/guidance_length_phased.c src/guidance_norm.c src/agc_multiword.c
build/sband-plane-native: tools/sband_plane_snapshot.c $(SBB_CORE)
	$(CC) -O2 $(SBB_FLAGS) $^ -o $@
build/sband-plane-reference: tools/sband_plane_reference.c $(SBB_INPUT)
	$(CC) -O2 $(filter-out -Wconversion,$(SBB_FLAGS)) -ffunction-sections -Wl,--gc-sections -Ideps/virtualagc/yaAGC $^ build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
all: build/sband-plane-native build/sband-plane-reference
test: all
	python3 tools/sband_plane_compare.py
sanitize:
	$(CC) -O1 -g $(SBB_FLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/sband_plane_snapshot.c $(SBB_CORE) -o build/sband-plane-sanitize
	python3 tools/sband_plane_compare.py sanitize
.PHONY: all test sanitize
