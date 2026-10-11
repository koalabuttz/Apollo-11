# SPDX-License-Identifier: MIT
CC ?= cc
MOS_CC ?= mos-c64-clang
VICE ?= x64sc
ROM_DIR ?= /workspace/toolchains/roms/C64
VICE_DATA_DIR ?= /workspace/toolchains/vice/usr/share/vice
LM_IB_CORE = tests/lm_interbank_target.c tests/lm_interbank_cases.c tests/frozen_words.c src/lm_interbank.c src/agc_word.c
LM_IB_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
.PHONY: all test sanitize check-host vice-test c64
all: test
test:
	$(CC) -O2 $(LM_IB_FLAGS) tests/test_lm_interbank.c $(LM_IB_CORE) -o build/test-lm-interbank
	./build/test-lm-interbank
sanitize:
	$(CC) -O1 -g $(LM_IB_FLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer tests/test_lm_interbank.c $(LM_IB_CORE) -o build/test-lm-interbank-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-interbank-sanitize
check-host:
	python3 tools/lm_interbank_host_test.py
c64 vice-test:
	python3 tools/lm_interbank_vice_test.py --cc $(MOS_CC) --vice $(VICE) --rom-dir $(ROM_DIR) --data-dir $(VICE_DATA_DIR)
