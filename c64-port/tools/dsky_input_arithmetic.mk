# SPDX-License-Identifier: MIT
DSKY_INPUT_ARITHMETIC_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_INPUT_ARITHMETIC_CORE = src/agc_word.c src/agc_multiword.c src/dsky_input_arithmetic.c tests/p66_bias_cases.c tests/dsky_input_arithmetic_cases.c
DSKY_INPUT_ARITHMETIC_TEST = tests/dsky_input_arithmetic_check.c tests/test_dsky_input_arithmetic.c
all: build/test-dsky-input-arithmetic
build/test-dsky-input-arithmetic: $(DSKY_INPUT_ARITHMETIC_CORE) $(DSKY_INPUT_ARITHMETIC_TEST) tests/dsky_input_arithmetic_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_INPUT_ARITHMETIC_FLAGS) $(DSKY_INPUT_ARITHMETIC_CORE) $(DSKY_INPUT_ARITHMETIC_TEST) -o $@
test-dsky-input-arithmetic: build/test-dsky-input-arithmetic
	build/test-dsky-input-arithmetic
sanitize-dsky-input-arithmetic:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_INPUT_ARITHMETIC_FLAGS) $(DSKY_INPUT_ARITHMETIC_CORE) $(DSKY_INPUT_ARITHMETIC_TEST) -o build/test-dsky-input-arithmetic-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-input-arithmetic-sanitize
c64-dsky-input-arithmetic:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_INPUT_ARITHMETIC_TARGET_LM $(DSKY_INPUT_ARITHMETIC_FLAGS) $(DSKY_INPUT_ARITHMETIC_CORE) tests/dsky_input_arithmetic_check.c tools/dsky_input_arithmetic_diagnostic.c -Wl,-Map,build/dsky_input_arithmetic-lm.map -o build/dsky_input_arithmetic-lm.prg
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_INPUT_ARITHMETIC_TARGET_CM $(DSKY_INPUT_ARITHMETIC_FLAGS) $(DSKY_INPUT_ARITHMETIC_CORE) tests/dsky_input_arithmetic_check.c tools/dsky_input_arithmetic_diagnostic.c -Wl,-Map,build/dsky_input_arithmetic-cm.map -o build/dsky_input_arithmetic-cm.prg
differential-dsky-input-arithmetic:
	python3 tools/dsky_input_arithmetic_differential.py
	python3 tools/freeze_dsky_input_arithmetic.py
test: test-dsky-input-arithmetic
sanitize: sanitize-dsky-input-arithmetic
.PHONY: all test sanitize test-dsky-input-arithmetic sanitize-dsky-input-arithmetic c64-dsky-input-arithmetic differential-dsky-input-arithmetic
