# SPDX-License-Identifier: MIT
CC ?= cc
MOS_CC ?= mos-c64-clang
VICE ?= x64sc
ROM_DIR ?= /workspace/toolchains/roms/C64
VICE_DATA_DIR ?= /workspace/toolchains/vice/usr/share/vice
CM_IC_CORE = tests/cm_interbank_complete_target.c tests/cm_interbank_complete_cases.c tests/frozen_words.c src/cm_interbank_complete.c src/agc_word.c
CM_IC_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
.PHONY: all test sanitize check-host vice-test c64
all: test
test:
	$(CC) -O2 $(CM_IC_FLAGS) tests/test_cm_interbank_complete.c $(CM_IC_CORE) -o build/test-cm-interbank-complete
	./build/test-cm-interbank-complete
sanitize:
	$(CC) -O1 -g $(CM_IC_FLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer tests/test_cm_interbank_complete.c $(CM_IC_CORE) -o build/test-cm-interbank-complete-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-interbank-complete-sanitize
check-host:
	python3 tools/cm_interbank_complete_host_test.py
c64 vice-test:
	python3 tools/cm_interbank_complete_vice_test.py --cc $(MOS_CC) --vice $(VICE) --rom-dir $(ROM_DIR) --data-dir $(VICE_DATA_DIR)
