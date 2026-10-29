# SPDX-License-Identifier: MIT
DSKY_REQUEST_CONTROL_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_REQUEST_CONTROL_CORE = src/agc_word.c src/dsky_request_control.c tests/p66_bias_cases.c tests/dsky_request_control_cases.c
DSKY_REQUEST_CONTROL_TEST = tests/dsky_request_control_check.c tests/test_dsky_request_control.c
all: build/test-dsky-request-control
build/test-dsky-request-control: $(DSKY_REQUEST_CONTROL_CORE) $(DSKY_REQUEST_CONTROL_TEST) tests/dsky_request_control_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_REQUEST_CONTROL_FLAGS) $(DSKY_REQUEST_CONTROL_CORE) $(DSKY_REQUEST_CONTROL_TEST) -o $@
test-dsky-request-control: build/test-dsky-request-control
	build/test-dsky-request-control
sanitize-dsky-request-control:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_REQUEST_CONTROL_FLAGS) $(DSKY_REQUEST_CONTROL_CORE) $(DSKY_REQUEST_CONTROL_TEST) -o build/test-dsky-request-control-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-request-control-sanitize
c64-dsky-request-control:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_REQUEST_CONTROL_FLAGS) $(DSKY_REQUEST_CONTROL_CORE) tests/dsky_request_control_check.c tools/dsky_request_control_diagnostic.c -Wl,-Map,build/dsky_request_control.map -o build/dsky_request_control.prg
differential-dsky-request-control:
	python3 tools/dsky_request_control_differential.py
	python3 tools/freeze_dsky_request_control.py
test: test-dsky-request-control
sanitize: sanitize-dsky-request-control
.PHONY: all test sanitize test-dsky-request-control sanitize-dsky-request-control c64-dsky-request-control differential-dsky-request-control
