# SPDX-License-Identifier: MIT
DSKY_INPUT_SELECT_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_INPUT_SELECT_CORE = src/agc_word.c src/dsky_input_select.c tests/p66_bias_cases.c tests/dsky_input_select_cases.c
DSKY_INPUT_SELECT_TEST = tests/dsky_input_select_check.c tests/test_dsky_input_select.c
all: build/test-dsky-input-select
build/test-dsky-input-select: $(DSKY_INPUT_SELECT_CORE) $(DSKY_INPUT_SELECT_TEST) tests/dsky_input_select_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_INPUT_SELECT_FLAGS) $(DSKY_INPUT_SELECT_CORE) $(DSKY_INPUT_SELECT_TEST) -o $@
test-dsky-input-select: build/test-dsky-input-select
	build/test-dsky-input-select
sanitize-dsky-input-select:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_INPUT_SELECT_FLAGS) $(DSKY_INPUT_SELECT_CORE) $(DSKY_INPUT_SELECT_TEST) -o build/test-dsky-input-select-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-input-select-sanitize
c64-dsky-input-select:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_INPUT_SELECT_TARGET_LM $(DSKY_INPUT_SELECT_FLAGS) $(DSKY_INPUT_SELECT_CORE) tests/dsky_input_select_check.c tools/dsky_input_select_diagnostic.c -Wl,-Map,build/dsky_input_select-lm.map -o build/dsky_input_select-lm.prg
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_INPUT_SELECT_TARGET_CM $(DSKY_INPUT_SELECT_FLAGS) $(DSKY_INPUT_SELECT_CORE) tests/dsky_input_select_check.c tools/dsky_input_select_diagnostic.c -Wl,-Map,build/dsky_input_select-cm.map -o build/dsky_input_select-cm.prg
differential-dsky-input-select:
	python3 tools/dsky_input_select_differential.py
	python3 tools/freeze_dsky_input_select.py
test: test-dsky-input-select
sanitize: sanitize-dsky-input-select
.PHONY: all test sanitize test-dsky-input-select sanitize-dsky-input-select c64-dsky-input-select differential-dsky-input-select
