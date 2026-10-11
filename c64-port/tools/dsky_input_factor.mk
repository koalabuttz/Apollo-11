# SPDX-License-Identifier: MIT
DSKY_INPUT_FACTOR_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_INPUT_FACTOR_CORE = src/agc_word.c src/noun_reader.c src/fixed_data.c src/dsky_input_select.c src/dsky_input_factor.c tests/p66_bias_cases.c tests/dsky_input_factor_cases.c
DSKY_INPUT_FACTOR_TEST = tests/dsky_input_factor_check.c tests/test_dsky_input_factor.c
all: build/test-dsky-input-factor
build/test-dsky-input-factor: $(DSKY_INPUT_FACTOR_CORE) $(DSKY_INPUT_FACTOR_TEST) tests/dsky_input_factor_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_INPUT_FACTOR_FLAGS) $(DSKY_INPUT_FACTOR_CORE) $(DSKY_INPUT_FACTOR_TEST) -o $@
test-dsky-input-factor: build/test-dsky-input-factor
	build/test-dsky-input-factor
sanitize-dsky-input-factor:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_INPUT_FACTOR_FLAGS) $(DSKY_INPUT_FACTOR_CORE) $(DSKY_INPUT_FACTOR_TEST) -o build/test-dsky-input-factor-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-input-factor-sanitize
c64-dsky-input-factor:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_INPUT_FACTOR_TARGET_LM -DAPOLLO_FIXED_LM_ONLY $(DSKY_INPUT_FACTOR_FLAGS) $(DSKY_INPUT_FACTOR_CORE) tests/dsky_input_factor_check.c tools/dsky_input_factor_diagnostic.c -Wl,-Map,build/dsky_input_factor-lm.map -o build/dsky_input_factor-lm.prg
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_INPUT_FACTOR_TARGET_CM -DAPOLLO_FIXED_CM_ONLY $(DSKY_INPUT_FACTOR_FLAGS) $(DSKY_INPUT_FACTOR_CORE) tests/dsky_input_factor_check.c tools/dsky_input_factor_diagnostic.c -Wl,-Map,build/dsky_input_factor-cm.map -o build/dsky_input_factor-cm.prg
differential-dsky-input-factor:
	python3 tools/dsky_input_factor_differential.py
	python3 tools/freeze_dsky_input_factor.py
test: test-dsky-input-factor
sanitize: sanitize-dsky-input-factor
.PHONY: all test sanitize test-dsky-input-factor sanitize-dsky-input-factor c64-dsky-input-factor differential-dsky-input-factor
