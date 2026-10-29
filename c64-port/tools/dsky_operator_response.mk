# SPDX-License-Identifier: MIT
DSKY_OPERATOR_RESPONSE_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_OPERATOR_RESPONSE_CORE = src/agc_word.c src/agc_multiword.c src/lm_interbank.c src/cm_interbank_complete.c src/executive.c src/p66_native.c src/lm_basic_wake.c src/cm_dispatch.c src/cm_executive.c src/cm_native_image.c src/cm_basic_wake.c src/dsky_display_release.c src/dsky_response_recall.c src/dsky_operator_response.c tests/p66_bias_cases.c tests/dsky_operator_response_cases.c
DSKY_OPERATOR_RESPONSE_TEST = tests/dsky_operator_response_check.c tests/test_dsky_operator_response.c
all: build/test-dsky-operator-response
build/test-dsky-operator-response: $(DSKY_OPERATOR_RESPONSE_CORE) $(DSKY_OPERATOR_RESPONSE_TEST) tests/dsky_operator_response_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_OPERATOR_RESPONSE_FLAGS) $(DSKY_OPERATOR_RESPONSE_CORE) $(DSKY_OPERATOR_RESPONSE_TEST) -o $@
test-dsky-operator-response: build/test-dsky-operator-response
	build/test-dsky-operator-response
sanitize-dsky-operator-response:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_OPERATOR_RESPONSE_FLAGS) $(DSKY_OPERATOR_RESPONSE_CORE) $(DSKY_OPERATOR_RESPONSE_TEST) -o build/test-dsky-operator-response-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-operator-response-sanitize
c64-dsky-operator-response:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_OPERATOR_RESPONSE_TARGET_LM $(DSKY_OPERATOR_RESPONSE_FLAGS) $(DSKY_OPERATOR_RESPONSE_CORE) tests/dsky_operator_response_check.c tools/dsky_operator_response_diagnostic.c -Wl,-Map,build/dsky_operator_response-lm.map -o build/dsky_operator_response-lm.prg
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_OPERATOR_RESPONSE_TARGET_CM $(DSKY_OPERATOR_RESPONSE_FLAGS) $(DSKY_OPERATOR_RESPONSE_CORE) tests/dsky_operator_response_check.c tools/dsky_operator_response_diagnostic.c -Wl,-Map,build/dsky_operator_response-cm.map -o build/dsky_operator_response-cm.prg
differential-dsky-operator-response:
	python3 tools/dsky_operator_response_differential.py
	python3 tools/freeze_dsky_operator_response.py
test: test-dsky-operator-response
sanitize: sanitize-dsky-operator-response
.PHONY: all test sanitize test-dsky-operator-response sanitize-dsky-operator-response c64-dsky-operator-response differential-dsky-operator-response
