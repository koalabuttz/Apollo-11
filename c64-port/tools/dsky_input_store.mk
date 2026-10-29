# SPDX-License-Identifier: MIT
DSKY_INPUT_STORE_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_INPUT_STORE_CORE = src/agc_word.c src/dsky_noun_check.c src/dsky_input_select.c src/dsky_input_store.c tests/p66_bias_cases.c tests/dsky_input_store_cases.c
DSKY_INPUT_STORE_TEST = tests/dsky_input_store_check.c tests/test_dsky_input_store.c
all: build/test-dsky-input-store
build/test-dsky-input-store: $(DSKY_INPUT_STORE_CORE) $(DSKY_INPUT_STORE_TEST) tests/dsky_input_store_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_INPUT_STORE_FLAGS) $(DSKY_INPUT_STORE_CORE) $(DSKY_INPUT_STORE_TEST) -o $@
test-dsky-input-store: build/test-dsky-input-store
	build/test-dsky-input-store
sanitize-dsky-input-store:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_INPUT_STORE_FLAGS) $(DSKY_INPUT_STORE_CORE) $(DSKY_INPUT_STORE_TEST) -o build/test-dsky-input-store-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-input-store-sanitize
c64-dsky-input-store:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_INPUT_STORE_TARGET_LM $(DSKY_INPUT_STORE_FLAGS) $(DSKY_INPUT_STORE_CORE) tests/dsky_input_store_check.c tools/dsky_input_store_diagnostic.c -Wl,-Map,build/dsky_input_store-lm.map -o build/dsky_input_store-lm.prg
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_INPUT_STORE_TARGET_CM $(DSKY_INPUT_STORE_FLAGS) $(DSKY_INPUT_STORE_CORE) tests/dsky_input_store_check.c tools/dsky_input_store_diagnostic.c -Wl,-Map,build/dsky_input_store-cm.map -o build/dsky_input_store-cm.prg
differential-dsky-input-store:
	python3 tools/dsky_input_store_differential.py
	python3 tools/freeze_dsky_input_store.py
test: test-dsky-input-store
sanitize: sanitize-dsky-input-store
.PHONY: all test sanitize test-dsky-input-store sanitize-dsky-input-store c64-dsky-input-store differential-dsky-input-store
