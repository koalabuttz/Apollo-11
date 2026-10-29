# SPDX-License-Identifier: MIT
DSKY_LOAD_STORE_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_LOAD_STORE_CORE = src/agc_word.c src/fixed_data.c src/noun_reader.c src/dsky_input_mode.c src/dsky_load_store.c tests/p66_bias_cases.c tests/dsky_load_store_cases.c
DSKY_LOAD_STORE_TEST = tests/dsky_load_store_check.c tests/test_dsky_load_store.c
all: build/test-dsky-load-store
build/test-dsky-load-store: $(DSKY_LOAD_STORE_CORE) $(DSKY_LOAD_STORE_TEST) tests/dsky_load_store_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_LOAD_STORE_FLAGS) $(DSKY_LOAD_STORE_CORE) $(DSKY_LOAD_STORE_TEST) -o $@
test-dsky-load-store: build/test-dsky-load-store
	build/test-dsky-load-store
sanitize-dsky-load-store:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_LOAD_STORE_FLAGS) $(DSKY_LOAD_STORE_CORE) $(DSKY_LOAD_STORE_TEST) -o build/test-dsky-load-store-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-load-store-sanitize
c64-dsky-load-store:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_LOAD_STORE_TARGET_LM -DAPOLLO_FIXED_LM_ONLY $(DSKY_LOAD_STORE_FLAGS) $(DSKY_LOAD_STORE_CORE) tests/dsky_load_store_check.c tools/dsky_load_store_diagnostic.c -Wl,-Map,build/dsky_load_store-lm.map -o build/dsky_load_store-lm.prg
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_LOAD_STORE_TARGET_CM -DAPOLLO_FIXED_CM_ONLY $(DSKY_LOAD_STORE_FLAGS) $(DSKY_LOAD_STORE_CORE) tests/dsky_load_store_check.c tools/dsky_load_store_diagnostic.c -Wl,-Map,build/dsky_load_store-cm.map -o build/dsky_load_store-cm.prg
differential-dsky-load-store:
	python3 tools/dsky_load_store_differential.py
	python3 tools/freeze_dsky_load_store.py
test: test-dsky-load-store
sanitize: sanitize-dsky-load-store
.PHONY: all test sanitize test-dsky-load-store sanitize-dsky-load-store c64-dsky-load-store differential-dsky-load-store
