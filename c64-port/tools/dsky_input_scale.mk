# SPDX-License-Identifier: MIT
DSKY_INPUT_SCALE_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_INPUT_SCALE_CORE = src/agc_word.c src/agc_multiword.c src/guidance_product.c src/dsky_input_arithmetic.c src/dsky_input_scale.c tests/p66_bias_cases.c tests/dsky_input_scale_cases.c
DSKY_INPUT_SCALE_TEST = tests/dsky_input_scale_check.c tests/test_dsky_input_scale.c
all: build/test-dsky-input-scale
build/test-dsky-input-scale: $(DSKY_INPUT_SCALE_CORE) $(DSKY_INPUT_SCALE_TEST) tests/dsky_input_scale_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_INPUT_SCALE_FLAGS) $(DSKY_INPUT_SCALE_CORE) $(DSKY_INPUT_SCALE_TEST) -o $@
test-dsky-input-scale: build/test-dsky-input-scale
	build/test-dsky-input-scale
sanitize-dsky-input-scale:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_INPUT_SCALE_FLAGS) $(DSKY_INPUT_SCALE_CORE) $(DSKY_INPUT_SCALE_TEST) -o build/test-dsky-input-scale-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-input-scale-sanitize
c64-dsky-input-scale:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_INPUT_SCALE_TARGET_LM $(DSKY_INPUT_SCALE_FLAGS) $(DSKY_INPUT_SCALE_CORE) tests/dsky_input_scale_check.c tools/dsky_input_scale_diagnostic.c -Wl,-Map,build/dsky_input_scale-lm.map -o build/dsky_input_scale-lm.prg
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_INPUT_SCALE_TARGET_CM $(DSKY_INPUT_SCALE_FLAGS) $(DSKY_INPUT_SCALE_CORE) tests/dsky_input_scale_check.c tools/dsky_input_scale_diagnostic.c -Wl,-Map,build/dsky_input_scale-cm.map -o build/dsky_input_scale-cm.prg
differential-dsky-input-scale:
	python3 tools/dsky_input_scale_differential.py
	python3 tools/freeze_dsky_input_scale.py
test: test-dsky-input-scale
sanitize: sanitize-dsky-input-scale
.PHONY: all test sanitize test-dsky-input-scale sanitize-dsky-input-scale c64-dsky-input-scale differential-dsky-input-scale
