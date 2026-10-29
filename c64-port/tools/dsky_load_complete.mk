# SPDX-License-Identifier: MIT
DSKY_LOAD_COMPLETE_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_LOAD_COMPLETE_CORE = src/agc_word.c src/agc_multiword.c src/lm_interbank.c src/cm_interbank_complete.c src/executive.c src/p66_native.c src/lm_basic_wake.c src/cm_dispatch.c src/cm_executive.c src/cm_native_image.c src/cm_basic_wake.c src/dsky_display_release.c src/dsky_response_recall.c src/dsky_load_complete.c tests/p66_bias_cases.c tests/dsky_load_complete_cases.c
DSKY_LOAD_COMPLETE_TEST = tests/dsky_load_complete_check.c tests/test_dsky_load_complete.c
all: build/test-dsky-load-complete
build/test-dsky-load-complete: $(DSKY_LOAD_COMPLETE_CORE) $(DSKY_LOAD_COMPLETE_TEST) tests/dsky_load_complete_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_LOAD_COMPLETE_FLAGS) $(DSKY_LOAD_COMPLETE_CORE) $(DSKY_LOAD_COMPLETE_TEST) -o $@
test-dsky-load-complete: build/test-dsky-load-complete
	build/test-dsky-load-complete
sanitize-dsky-load-complete:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_LOAD_COMPLETE_FLAGS) $(DSKY_LOAD_COMPLETE_CORE) $(DSKY_LOAD_COMPLETE_TEST) -o build/test-dsky-load-complete-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-load-complete-sanitize
c64-dsky-load-complete:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_LOAD_COMPLETE_TARGET_LM $(DSKY_LOAD_COMPLETE_FLAGS) $(DSKY_LOAD_COMPLETE_CORE) tests/dsky_load_complete_check.c tools/dsky_load_complete_diagnostic.c -Wl,-Map,build/dsky_load_complete-lm.map -o build/dsky_load_complete-lm.prg
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_LOAD_COMPLETE_TARGET_CM $(DSKY_LOAD_COMPLETE_FLAGS) $(DSKY_LOAD_COMPLETE_CORE) tests/dsky_load_complete_check.c tools/dsky_load_complete_diagnostic.c -Wl,-Map,build/dsky_load_complete-cm.map -o build/dsky_load_complete-cm.prg
differential-dsky-load-complete:
	python3 tools/dsky_load_complete_differential.py
	python3 tools/freeze_dsky_load_complete.py
test: test-dsky-load-complete
sanitize: sanitize-dsky-load-complete
.PHONY: all test sanitize test-dsky-load-complete sanitize-dsky-load-complete c64-dsky-load-complete differential-dsky-load-complete
