# SPDX-License-Identifier: MIT
CC ?= cc
LM_BASIC_CHANGE_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
LM_BASIC_CHANGE_SOURCES = src/agc_word.c src/lm_basic_change.c tests/lm_basic_change_cases.c
all: build/test-lm-basic-change
build:
	mkdir -p build
build/test-lm-basic-change: $(LM_BASIC_CHANGE_SOURCES) tests/test_lm_basic_change.c | build
	$(CC) $(LM_BASIC_CHANGE_FLAGS) -O2 $^ -o $@
test: all
	./build/test-lm-basic-change
	python3 tools/lm_basic_change_differential.py --retained
sanitize: | build
	$(CC) $(LM_BASIC_CHANGE_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(LM_BASIC_CHANGE_SOURCES) tests/test_lm_basic_change.c -o build/test-lm-basic-change-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-basic-change-sanitize
	python3 tools/lm_basic_change_differential.py --retained --sanitize
reference:
	python3 tools/lm_basic_change_differential.py
c64:
	python3 tools/lm_basic_change_target_build.py --cc "$(MOS_CC)" --c1541 "$(C1541)"
vice-test:
	python3 tools/lm_basic_change_vice_test.py --vice "$(VICE)" --rom-dir "$(C64_ROM_DIR)" --data-dir "$(VICE_DATA_DIR)"
.PHONY: all test sanitize reference c64 vice-test
