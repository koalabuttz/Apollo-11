# SPDX-License-Identifier: MIT
CC ?= cc
SBT_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
SBT_INPUT = tests/sband_trig_cases.c src/agc_word.c
SBT_CORE = $(SBT_INPUT) src/sband_trig.c src/agc_multiword.c src/guidance_product.c src/guidance_trig.c
all: build/sband-trig-native build/sband-trig-reference
build/sband-trig-native: tools/sband_trig_snapshot.c $(SBT_CORE)
	$(CC) -O2 $(SBT_FLAGS) $^ -o $@
build/sband-trig-reference: tools/sband_trig_reference.c $(SBT_INPUT)
	$(CC) -O2 $(filter-out -Wconversion,$(SBT_FLAGS)) -ffunction-sections -Wl,--gc-sections -Ideps/virtualagc/yaAGC $^ build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	python3 tools/sband_trig_compare.py
sanitize:
	$(CC) -O1 -g $(SBT_FLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer tools/sband_trig_snapshot.c $(SBT_CORE) -o build/sband-trig-sanitize
	python3 tools/sband_trig_compare.py sanitize
.PHONY: all test sanitize
build/sband-trig-inputs: tools/sband_trig_inputs.c $(SBT_INPUT)
	$(CC) -O2 $(SBT_FLAGS) $^ -o $@
freeze: build/sband-trig-inputs
	python3 tools/freeze_sband_trig.py
check-host:
	$(CC) -O2 $(SBT_FLAGS) tests/test_sband_trig.c tests/sband_trig_target.c tests/frozen_words.c $(SBT_CORE) -o build/test-sband-trig
	./build/test-sband-trig
.PHONY: freeze check-host
MOS_CC ?= mos-c64-clang
VICE ?= x64sc
ROM_DIR ?= /workspace/toolchains/roms/C64
VICE_DATA_DIR ?= /workspace/toolchains/vice/usr/share/vice
source-audit:
	python3 tools/sband_trig_source.py
vice-test:
	python3 tools/sband_trig_vice_test.py --cc $(MOS_CC) --vice $(VICE) --rom-dir $(ROM_DIR) --data-dir $(VICE_DATA_DIR)
.PHONY: source-audit vice-test
