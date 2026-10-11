# SPDX-License-Identifier: MIT
CC ?= cc
LM_ADMIT_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
LM_ADMIT_SOURCES = src/agc_word.c src/executive.c src/p66_native.c src/lm_admission_image.c tests/lm_admission_image_cases.c
all: build/test-lm-admission-image
build:
	mkdir -p build
build/test-lm-admission-image: $(LM_ADMIT_SOURCES) tests/test_lm_admission_image.c | build
	$(CC) $(LM_ADMIT_FLAGS) -O2 $^ -o $@
test: all
	./build/test-lm-admission-image
	python3 tools/lm_admission_image_differential.py --retained
sanitize: | build
	$(CC) $(LM_ADMIT_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(LM_ADMIT_SOURCES) tests/test_lm_admission_image.c -o build/test-lm-admission-image-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-admission-image-sanitize
	python3 tools/lm_admission_image_differential.py --retained --sanitize
reference:
	python3 tools/lm_admission_image_differential.py
c64:
	python3 tools/lm_admission_image_target_build.py --cc "$(MOS_CC)" --c1541 "$(C1541)"
vice-test:
	python3 tools/lm_admission_image_vice_test.py --vice "$(VICE)" --rom-dir "$(C64_ROM_DIR)" --data-dir "$(VICE_DATA_DIR)"
.PHONY: all test sanitize reference c64 vice-test
