# SPDX-License-Identifier: MIT
LM_BASIC_CHANGE_INTEGRATION_CORE = src/agc_word.c src/lm_basic_change.c tests/lm_basic_change_cases.c
LM_BASIC_CHANGE_INTEGRATION_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
all: build/test-lm-basic-change-integrated
build/test-lm-basic-change-integrated: $(LM_BASIC_CHANGE_INTEGRATION_CORE) tests/test_lm_basic_change.c | build
	$(CC) $(LM_BASIC_CHANGE_INTEGRATION_FLAGS) -O2 $^ -o $@
test: test-lm-basic-change-integrated
test-lm-basic-change-integrated: build/test-lm-basic-change-integrated
	./build/test-lm-basic-change-integrated
	python3 tools/lm_basic_change_differential.py --retained
sanitize: sanitize-lm-basic-change-integrated
sanitize-lm-basic-change-integrated: | build
	$(CC) $(LM_BASIC_CHANGE_INTEGRATION_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(LM_BASIC_CHANGE_INTEGRATION_CORE) tests/test_lm_basic_change.c -o build/test-lm-basic-change-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-basic-change-integrated-san
	python3 tools/lm_basic_change_differential.py --retained --sanitize
.PHONY: test-lm-basic-change-integrated sanitize-lm-basic-change-integrated
