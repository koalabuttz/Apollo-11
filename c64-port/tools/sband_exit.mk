# SPDX-License-Identifier: MIT
CC ?= cc
SBEXIT_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
SBEXIT_INPUT = tests/sband_exit_cases.c src/agc_word.c
SBEXIT_CORE = $(SBEXIT_INPUT) src/sband_exit.c
all: build/sband-exit-native build/sband-exit-reference
build/sband-exit-native: tools/sband_exit_snapshot.c $(SBEXIT_CORE)
	$(CC) -O2 $(SBEXIT_FLAGS) $^ -o $@
build/sband-exit-reference: tools/sband_exit_reference.c $(SBEXIT_INPUT)
	$(CC) -O2 $(filter-out -Wconversion,$(SBEXIT_FLAGS)) -ffunction-sections -Wl,--gc-sections -Ideps/virtualagc/yaAGC $^ build/agc_engine_hardware_dv.o deps/virtualagc/yaAGC/agc_engine_init.o deps/virtualagc/yaAGC/rfopen.o -o $@
test: all
	python3 tools/sband_exit_compare.py
sanitize:
	$(CC) -O1 -g $(SBEXIT_FLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/sband_exit_snapshot.c $(SBEXIT_CORE) -o build/sband-exit-sanitize
	python3 tools/sband_exit_compare.py sanitize
.PHONY: all test sanitize
