# SPDX-License-Identifier: MIT
LM_WAITLIST_IMAGE_INTEGRATION_CORE = src/agc_word.c src/waitlist.c src/lm_waitlist_image.c tests/lm_waitlist_image_cases.c
LM_WAITLIST_IMAGE_INTEGRATION_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
all: build/test-lm-waitlist-image-integrated
build/test-lm-waitlist-image-integrated: $(LM_WAITLIST_IMAGE_INTEGRATION_CORE) tests/test_lm_waitlist_image.c | build
	$(CC) $(LM_WAITLIST_IMAGE_INTEGRATION_FLAGS) -O2 $^ -o $@
test: test-lm-waitlist-image-integrated
test-lm-waitlist-image-integrated: build/test-lm-waitlist-image-integrated
	./build/test-lm-waitlist-image-integrated
	python3 tools/lm_waitlist_image_differential.py --retained
sanitize: sanitize-lm-waitlist-image-integrated
sanitize-lm-waitlist-image-integrated: | build
	$(CC) $(LM_WAITLIST_IMAGE_INTEGRATION_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(LM_WAITLIST_IMAGE_INTEGRATION_CORE) tests/test_lm_waitlist_image.c -o build/test-lm-waitlist-image-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-waitlist-image-integrated-san
	python3 tools/lm_waitlist_image_differential.py --retained --sanitize
.PHONY: test-lm-waitlist-image-integrated sanitize-lm-waitlist-image-integrated
