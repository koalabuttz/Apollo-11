# SPDX-License-Identifier: MIT
DSKY_OPERATOR_REQUEST_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_OPERATOR_REQUEST_CORE = src/agc_word.c src/executive.c src/lm_basic_wake.c src/agc_multiword.c src/cm_dispatch.c src/cm_executive.c src/cm_native_image.c src/cm_basic_wake.c src/dsky_display_release.c src/dsky_request_control.c src/dsky_operator_request.c tests/p66_bias_cases.c tests/dsky_operator_request_cases.c
DSKY_OPERATOR_REQUEST_TEST = tests/dsky_operator_request_check.c tests/test_dsky_operator_request.c
all: build/test-dsky-operator-request
build/test-dsky-operator-request: $(DSKY_OPERATOR_REQUEST_CORE) $(DSKY_OPERATOR_REQUEST_TEST) tests/dsky_operator_request_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_OPERATOR_REQUEST_FLAGS) $(DSKY_OPERATOR_REQUEST_CORE) $(DSKY_OPERATOR_REQUEST_TEST) -o $@
test-dsky-operator-request: build/test-dsky-operator-request
	build/test-dsky-operator-request
sanitize-dsky-operator-request:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_OPERATOR_REQUEST_FLAGS) $(DSKY_OPERATOR_REQUEST_CORE) $(DSKY_OPERATOR_REQUEST_TEST) -o build/test-dsky-operator-request-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-operator-request-sanitize
c64-dsky-operator-request:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_OPERATOR_REQUEST_FLAGS) $(DSKY_OPERATOR_REQUEST_CORE) tests/dsky_operator_request_check.c tools/dsky_operator_request_diagnostic.c -Wl,-Map,build/dsky_operator_request.map -o build/dsky_operator_request.prg
differential-dsky-operator-request:
	python3 tools/dsky_operator_request_differential.py
	python3 tools/freeze_dsky_operator_request.py
test: test-dsky-operator-request
sanitize: sanitize-dsky-operator-request
.PHONY: all test sanitize test-dsky-operator-request sanitize-dsky-operator-request c64-dsky-operator-request differential-dsky-operator-request
