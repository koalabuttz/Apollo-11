# SPDX-License-Identifier: MIT
DSKY_INPUT_MODE_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_INPUT_MODE_CORE = src/agc_word.c src/dsky_input_mode.c tests/p66_bias_cases.c tests/dsky_input_mode_cases.c
DSKY_INPUT_MODE_TEST = tests/dsky_input_mode_check.c tests/test_dsky_input_mode.c
all: build/test-dsky-input-mode
build/test-dsky-input-mode: $(DSKY_INPUT_MODE_CORE) $(DSKY_INPUT_MODE_TEST) tests/dsky_input_mode_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_INPUT_MODE_FLAGS) $(DSKY_INPUT_MODE_CORE) $(DSKY_INPUT_MODE_TEST) -o $@
test-dsky-input-mode: build/test-dsky-input-mode
	build/test-dsky-input-mode
sanitize-dsky-input-mode:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_INPUT_MODE_FLAGS) $(DSKY_INPUT_MODE_CORE) $(DSKY_INPUT_MODE_TEST) -o build/test-dsky-input-mode-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-input-mode-sanitize
c64-dsky-input-mode:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_INPUT_MODE_TARGET_LM $(DSKY_INPUT_MODE_FLAGS) $(DSKY_INPUT_MODE_CORE) tests/dsky_input_mode_check.c tools/dsky_input_mode_diagnostic.c -Wl,-Map,build/dsky_input_mode-lm.map -o build/dsky_input_mode-lm.prg
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_INPUT_MODE_TARGET_CM $(DSKY_INPUT_MODE_FLAGS) $(DSKY_INPUT_MODE_CORE) tests/dsky_input_mode_check.c tools/dsky_input_mode_diagnostic.c -Wl,-Map,build/dsky_input_mode-cm.map -o build/dsky_input_mode-cm.prg
differential-dsky-input-mode:
	python3 tools/dsky_input_mode_differential.py
	python3 tools/freeze_dsky_input_mode.py
test: test-dsky-input-mode
sanitize: sanitize-dsky-input-mode
.PHONY: all test sanitize test-dsky-input-mode sanitize-dsky-input-mode c64-dsky-input-mode differential-dsky-input-mode
