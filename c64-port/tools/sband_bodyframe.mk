# SPDX-License-Identifier: MIT
CC ?= cc
SBB_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
SBB_INPUT = tests/sband_bodyframe_cases.c src/agc_word.c
SBB_CORE = $(SBB_INPUT) src/sband_bodyframe.c src/lm_powered_interface.c src/lm_powered_axis.c src/cm_powered_interface.c src/cm_powered_axis.c src/guidance_rotation.c src/cm_rotation.c src/guidance_product.c src/agc_multiword.c
build/sband-bodyframe-native: tools/sband_bodyframe_snapshot.c $(SBB_CORE)
	$(CC) -O2 $(SBB_FLAGS) $^ -o $@
build/sband-bodyframe-reference: tools/sband_bodyframe_reference.c $(SBB_INPUT)
	$(CC) -O2 $(filter-out -Wconversion,$(SBB_FLAGS)) -ffunction-sections -Wl,--gc-sections -Ideps/virtualagc/yaAGC $^ build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
all: build/sband-bodyframe-native build/sband-bodyframe-reference
test: all
	python3 tools/sband_bodyframe_compare.py
sanitize:
	$(CC) -O1 -g $(SBB_FLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/sband_bodyframe_snapshot.c $(SBB_CORE) -o build/sband-bodyframe-sanitize
	python3 tools/sband_bodyframe_compare.py sanitize
.PHONY: all test sanitize
