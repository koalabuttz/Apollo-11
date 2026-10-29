# SPDX-License-Identifier: MIT
LM_ADMISSION_IMAGE_INTEGRATION_CORE = src/agc_word.c src/executive.c src/p66_native.c src/lm_admission_image.c tests/lm_admission_image_cases.c
LM_ADMISSION_IMAGE_INTEGRATION_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
all: build/test-lm-admission-image-integrated
build/test-lm-admission-image-integrated: $(LM_ADMISSION_IMAGE_INTEGRATION_CORE) tests/test_lm_admission_image.c | build
	$(CC) $(LM_ADMISSION_IMAGE_INTEGRATION_FLAGS) -O2 $^ -o $@
test: test-lm-admission-image-integrated
test-lm-admission-image-integrated: build/test-lm-admission-image-integrated
	./build/test-lm-admission-image-integrated
	python3 tools/lm_admission_image_differential.py --retained
sanitize: sanitize-lm-admission-image-integrated
sanitize-lm-admission-image-integrated: | build
	$(CC) $(LM_ADMISSION_IMAGE_INTEGRATION_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(LM_ADMISSION_IMAGE_INTEGRATION_CORE) tests/test_lm_admission_image.c -o build/test-lm-admission-image-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-admission-image-integrated-san
	python3 tools/lm_admission_image_differential.py --retained --sanitize
.PHONY: test-lm-admission-image-integrated sanitize-lm-admission-image-integrated
