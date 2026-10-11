# SPDX-License-Identifier: MIT
CC ?= cc
SBF_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
SBF_INPUT = tests/sband_flow_cases.c src/agc_word.c
SBF_CORE = $(SBF_INPUT) src/sband_flow.c src/lm_interbank.c src/cm_interbank_complete.c src/rtb_angles.c src/cm_powered_interface.c src/cm_powered_trig.c src/agc_multiword.c src/guidance_product.c src/guidance_trig.c
build/sband-flow-native: tools/sband_flow_snapshot.c $(SBF_CORE)
	$(CC) -O2 $(SBF_FLAGS) $^ -o $@
build/sband-flow-reference: tools/sband_flow_reference.c $(SBF_INPUT)
	$(CC) -O2 $(filter-out -Wconversion,$(SBF_FLAGS)) -ffunction-sections -Wl,--gc-sections -Ideps/virtualagc/yaAGC $^ build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
all: build/sband-flow-reference build/sband-flow-native
test: all
	python3 tools/sband_flow_compare.py
sanitize:
	$(CC) -O1 -g $(SBF_FLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/sband_flow_snapshot.c $(SBF_CORE) -o build/sband-flow-sanitize
	python3 tools/sband_flow_compare.py sanitize
build/sband-flow-inputs: tools/sband_flow_inputs.c $(SBF_INPUT)
	$(CC) -O2 $(SBF_FLAGS) $^ -o $@
freeze: build/sband-flow-inputs
	python3 tools/freeze_sband_flow.py
check-host:
	$(CC) -O2 $(SBF_FLAGS) tests/test_sband_flow.c tests/sband_flow_target.c tests/frozen_words.c $(SBF_CORE) -o build/test-sband-flow
	./build/test-sband-flow
MOS_CC ?= mos-c64-clang
VICE ?= x64sc
ROM_DIR ?= /workspace/toolchains/roms/C64
VICE_DATA_DIR ?= /workspace/toolchains/vice/usr/share/vice
vice-test:
	python3 tools/sband_flow_vice_test.py --cc $(MOS_CC) --vice $(VICE) --rom-dir $(ROM_DIR) --data-dir $(VICE_DATA_DIR)
.PHONY: all test sanitize freeze check-host vice-test
