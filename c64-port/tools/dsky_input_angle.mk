# SPDX-License-Identifier: MIT
DSKY_INPUT_ANGLE_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_INPUT_ANGLE_CORE = src/agc_word.c src/agc_multiword.c src/guidance_product.c src/dsky_input_arithmetic.c src/dsky_input_select.c src/dsky_input_angle.c tests/p66_bias_cases.c tests/dsky_input_angle_cases.c
DSKY_INPUT_ANGLE_TEST = tests/dsky_input_angle_check.c tests/test_dsky_input_angle.c
all: build/test-dsky-input-angle
build/test-dsky-input-angle: $(DSKY_INPUT_ANGLE_CORE) $(DSKY_INPUT_ANGLE_TEST) tests/dsky_input_angle_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_INPUT_ANGLE_FLAGS) $(DSKY_INPUT_ANGLE_CORE) $(DSKY_INPUT_ANGLE_TEST) -o $@
test-dsky-input-angle: build/test-dsky-input-angle
	build/test-dsky-input-angle
sanitize-dsky-input-angle:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_INPUT_ANGLE_FLAGS) $(DSKY_INPUT_ANGLE_CORE) $(DSKY_INPUT_ANGLE_TEST) -o build/test-dsky-input-angle-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-input-angle-sanitize
c64-dsky-input-angle:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_INPUT_ANGLE_TARGET_LM $(DSKY_INPUT_ANGLE_FLAGS) $(DSKY_INPUT_ANGLE_CORE) tests/dsky_input_angle_check.c tools/dsky_input_angle_diagnostic.c -Wl,-Map,build/dsky_input_angle-lm.map -o build/dsky_input_angle-lm.prg
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_INPUT_ANGLE_TARGET_CM $(DSKY_INPUT_ANGLE_FLAGS) $(DSKY_INPUT_ANGLE_CORE) tests/dsky_input_angle_check.c tools/dsky_input_angle_diagnostic.c -Wl,-Map,build/dsky_input_angle-cm.map -o build/dsky_input_angle-cm.prg
differential-dsky-input-angle:
	python3 tools/dsky_input_angle_differential.py
	python3 tools/freeze_dsky_input_angle.py
test: test-dsky-input-angle
sanitize: sanitize-dsky-input-angle
.PHONY: all test sanitize test-dsky-input-angle sanitize-dsky-input-angle c64-dsky-input-angle differential-dsky-input-angle
