# SPDX-License-Identifier: MIT
LM_DANZIG_CHANGE_INTEGRATION_CORE = src/agc_word.c src/lm_interpreter_change.c src/lm_danzig_change.c tests/lm_interpreter_change_cases.c tests/lm_danzig_change_cases.c
LM_DANZIG_CHANGE_INTEGRATION_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
all: build/test-lm-danzig-change-integrated
build/test-lm-danzig-change-integrated: $(LM_DANZIG_CHANGE_INTEGRATION_CORE) tests/test_lm_danzig_change.c | build
	$(CC) $(LM_DANZIG_CHANGE_INTEGRATION_FLAGS) -O2 $^ -o $@
test: test-lm-danzig-change-integrated
test-lm-danzig-change-integrated: build/test-lm-danzig-change-integrated
	./build/test-lm-danzig-change-integrated
	python3 tools/lm_danzig_change_differential.py --retained
sanitize: sanitize-lm-danzig-change-integrated
sanitize-lm-danzig-change-integrated: | build
	$(CC) $(LM_DANZIG_CHANGE_INTEGRATION_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(LM_DANZIG_CHANGE_INTEGRATION_CORE) tests/test_lm_danzig_change.c -o build/test-lm-danzig-change-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-danzig-change-integrated-san
	python3 tools/lm_danzig_change_differential.py --retained --sanitize
.PHONY: test-lm-danzig-change-integrated sanitize-lm-danzig-change-integrated
