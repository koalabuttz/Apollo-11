# SPDX-License-Identifier: MIT
DSKY_RESPONSE_RECALL_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_RESPONSE_RECALL_CORE = src/agc_word.c src/agc_multiword.c src/lm_interbank.c src/cm_interbank_complete.c src/executive.c src/p66_native.c src/lm_basic_wake.c src/cm_dispatch.c src/cm_executive.c src/cm_native_image.c src/cm_basic_wake.c src/dsky_display_release.c src/dsky_response_recall.c tests/p66_bias_cases.c tests/dsky_response_recall_cases.c
DSKY_RESPONSE_RECALL_TEST = tests/dsky_response_recall_check.c tests/test_dsky_response_recall.c
all: build/test-dsky-response-recall
build/test-dsky-response-recall: $(DSKY_RESPONSE_RECALL_CORE) $(DSKY_RESPONSE_RECALL_TEST) tests/dsky_response_recall_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_RESPONSE_RECALL_FLAGS) $(DSKY_RESPONSE_RECALL_CORE) $(DSKY_RESPONSE_RECALL_TEST) -o $@
test-dsky-response-recall: build/test-dsky-response-recall
	build/test-dsky-response-recall
sanitize-dsky-response-recall:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_RESPONSE_RECALL_FLAGS) $(DSKY_RESPONSE_RECALL_CORE) $(DSKY_RESPONSE_RECALL_TEST) -o build/test-dsky-response-recall-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-response-recall-sanitize
c64-dsky-response-recall:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_RESPONSE_RECALL_TARGET_LM $(DSKY_RESPONSE_RECALL_FLAGS) $(DSKY_RESPONSE_RECALL_CORE) tests/dsky_response_recall_check.c tools/dsky_response_recall_diagnostic.c -Wl,-Map,build/dsky_response_recall-lm.map -o build/dsky_response_recall-lm.prg
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_RESPONSE_RECALL_TARGET_CM $(DSKY_RESPONSE_RECALL_FLAGS) $(DSKY_RESPONSE_RECALL_CORE) tests/dsky_response_recall_check.c tools/dsky_response_recall_diagnostic.c -Wl,-Map,build/dsky_response_recall-cm.map -o build/dsky_response_recall-cm.prg
differential-dsky-response-recall:
	python3 tools/dsky_response_recall_differential.py
	python3 tools/freeze_dsky_response_recall.py
test: test-dsky-response-recall
sanitize: sanitize-dsky-response-recall
.PHONY: all test sanitize test-dsky-response-recall sanitize-dsky-response-recall c64-dsky-response-recall differential-dsky-response-recall
