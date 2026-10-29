# SPDX-License-Identifier: MIT
CC ?= cc
SBS_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
SBS_INPUT = tests/sband_linear_cases.c src/agc_word.c
SBS_CORE = $(SBS_INPUT) src/sband_vector_linear.c src/agc_multiword.c
build/sband-linear-native: tools/sband_linear_snapshot.c $(SBS_CORE)
	$(CC) -O2 $(SBS_FLAGS) $^ -o $@
build/sband-linear-reference: tools/sband_linear_reference.c $(SBS_INPUT)
	$(CC) -O2 $(filter-out -Wconversion,$(SBS_FLAGS)) -ffunction-sections -Wl,--gc-sections -Ideps/virtualagc/yaAGC $^ build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
all: build/sband-linear-native build/sband-linear-reference
test: all
	python3 tools/sband_linear_compare.py
sanitize:
	$(CC) -O1 -g $(SBS_FLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/sband_linear_snapshot.c $(SBS_CORE) -o build/sband-linear-sanitize
	python3 tools/sband_linear_compare.py sanitize
.PHONY: all test sanitize
build/sband-linear-inputs: tools/sband_linear_inputs.c $(SBS_INPUT)
	$(CC) -O2 $(SBS_FLAGS) $^ -o $@
freeze: build/sband-linear-inputs
	python3 tools/freeze_sband_linear.py
vice-test:
	python3 tools/sband_linear_vice_test.py --cc /workspace/toolchains/llvm-mos/bin/mos-c64-clang --vice /workspace/toolchains/vice/usr/bin/x64sc --rom-dir /workspace/toolchains/roms/C64 --data-dir /workspace/toolchains/vice/usr/share/vice
.PHONY: freeze vice-test
check-host:
	$(CC) -O2 $(SBS_FLAGS) tests/test_sband_linear.c tests/sband_linear_target.c tests/frozen_words.c $(SBS_CORE) -o build/test-sband-linear
	./build/test-sband-linear
.PHONY: check-host
