# SPDX-License-Identifier: MIT
DSKY_INPUT_TIME_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_INPUT_TIME_CORE = src/agc_word.c src/agc_multiword.c src/guidance_product.c src/dsky_input_mode.c src/dsky_input_time.c tests/p66_bias_cases.c tests/dsky_input_time_cases.c
DSKY_INPUT_TIME_TEST = tests/dsky_input_time_check.c tests/test_dsky_input_time.c
all: build/test-dsky-input-time
build/test-dsky-input-time: $(DSKY_INPUT_TIME_CORE) $(DSKY_INPUT_TIME_TEST) tests/dsky_input_time_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_INPUT_TIME_FLAGS) $(DSKY_INPUT_TIME_CORE) $(DSKY_INPUT_TIME_TEST) -o $@
test-dsky-input-time: build/test-dsky-input-time
	build/test-dsky-input-time
sanitize-dsky-input-time:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_INPUT_TIME_FLAGS) $(DSKY_INPUT_TIME_CORE) $(DSKY_INPUT_TIME_TEST) -o build/test-dsky-input-time-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-input-time-sanitize
c64-dsky-input-time:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_INPUT_TIME_TARGET_LM $(DSKY_INPUT_TIME_FLAGS) $(DSKY_INPUT_TIME_CORE) tests/dsky_input_time_check.c tools/dsky_input_time_diagnostic.c -Wl,-Map,build/dsky_input_time-lm.map -o build/dsky_input_time-lm.prg
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_INPUT_TIME_TARGET_CM $(DSKY_INPUT_TIME_FLAGS) $(DSKY_INPUT_TIME_CORE) tests/dsky_input_time_check.c tools/dsky_input_time_diagnostic.c -Wl,-Map,build/dsky_input_time-cm.map -o build/dsky_input_time-cm.prg
differential-dsky-input-time:
	python3 tools/dsky_input_time_differential.py
	python3 tools/freeze_dsky_input_time.py
test: test-dsky-input-time
sanitize: sanitize-dsky-input-time
.PHONY: all test sanitize test-dsky-input-time sanitize-dsky-input-time c64-dsky-input-time differential-dsky-input-time
