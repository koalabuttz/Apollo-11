# SPDX-License-Identifier: MIT
DSKY_MAJOR_REQUEST_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_MAJOR_REQUEST_CORE = src/agc_word.c src/dsky_pinball.c src/dsky_major_request.c tests/p66_bias_cases.c tests/dsky_major_request_cases.c
DSKY_MAJOR_REQUEST_TEST = tests/dsky_major_request_check.c tests/test_dsky_major_request.c
all: build/test-dsky-major-request
build/test-dsky-major-request: $(DSKY_MAJOR_REQUEST_CORE) $(DSKY_MAJOR_REQUEST_TEST) tests/dsky_major_request_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_MAJOR_REQUEST_FLAGS) $(DSKY_MAJOR_REQUEST_CORE) $(DSKY_MAJOR_REQUEST_TEST) -o $@
test-dsky-major-request: build/test-dsky-major-request
	build/test-dsky-major-request
sanitize-dsky-major-request:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_MAJOR_REQUEST_FLAGS) $(DSKY_MAJOR_REQUEST_CORE) $(DSKY_MAJOR_REQUEST_TEST) -o build/test-dsky-major-request-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-major-request-sanitize
c64-dsky-major-request:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_MAJOR_REQUEST_FLAGS) $(DSKY_MAJOR_REQUEST_CORE) tests/dsky_major_request_check.c tools/dsky_major_request_diagnostic.c -Wl,-Map,build/dsky_major_request.map -o build/dsky_major_request.prg
differential-dsky-major-request:
	python3 tools/dsky_major_request_differential.py
	python3 tools/freeze_dsky_major_request.py
test: test-dsky-major-request
sanitize: sanitize-dsky-major-request
.PHONY: all test sanitize test-dsky-major-request sanitize-dsky-major-request c64-dsky-major-request differential-dsky-major-request
