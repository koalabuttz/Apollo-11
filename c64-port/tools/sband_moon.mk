# SPDX-License-Identifier: MIT
CC ?= cc
SBO_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
SBO_INPUT = tests/sband_moon_cases.c src/agc_word.c
SBO_CORE = $(SBO_INPUT) src/sband_moon.c src/sband_vector_linear.c src/sband_shift.c src/guidance_shift.c src/agc_multiword.c
build/sband-moon-native: tools/sband_moon_snapshot.c $(SBO_CORE)
	$(CC) -O2 $(SBO_FLAGS) $^ -o $@
build/sband-moon-reference: tools/sband_moon_reference.c $(SBO_INPUT)
	$(CC) -O2 $(filter-out -Wconversion,$(SBO_FLAGS)) -ffunction-sections -Wl,--gc-sections -Ideps/virtualagc/yaAGC $^ build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
all: build/sband-moon-reference build/sband-moon-native
test: all
	python3 tools/sband_moon_compare.py
sanitize:
	$(CC) -O1 -g $(SBO_FLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/sband_moon_snapshot.c $(SBO_CORE) -o build/sband-moon-sanitize
	python3 tools/sband_moon_compare.py sanitize
.PHONY: all test sanitize
build/sband-moon-inputs: tools/sband_moon_inputs.c $(SBO_INPUT)
	$(CC) -O2 $(SBO_FLAGS) $^ -o $@
freeze: build/sband-moon-inputs
	python3 tools/freeze_sband_moon.py
check-host:
	$(CC) -O2 $(SBO_FLAGS) tests/test_sband_moon.c tests/sband_moon_target.c tests/frozen_words.c $(SBO_CORE) -o build/test-sband-moon
	./build/test-sband-moon
vice-test:
	python3 tools/sband_moon_vice_test.py --cc /workspace/toolchains/llvm-mos/bin/mos-c64-clang --vice /workspace/toolchains/vice/usr/bin/x64sc --rom-dir /workspace/toolchains/roms/C64 --data-dir /workspace/toolchains/vice/usr/share/vice
.PHONY: freeze check-host vice-test
