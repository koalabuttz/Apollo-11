# SPDX-License-Identifier: MIT
DSKY_ADDRESS_REQUEST_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_ADDRESS_REQUEST_CORE = src/agc_word.c src/dsky_pinball.c src/dsky_octal_display.c src/dsky_machine_cadr.c src/dsky_five_blank.c src/fixed_data.c src/noun_reader.c src/dsky_address_request.c tests/p66_bias_cases.c tests/dsky_address_request_cases.c
DSKY_ADDRESS_REQUEST_TEST = tests/dsky_address_request_check.c tests/test_dsky_address_request.c
all: build/test-dsky-address-request
build/test-dsky-address-request: $(DSKY_ADDRESS_REQUEST_CORE) $(DSKY_ADDRESS_REQUEST_TEST) tests/dsky_address_request_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_ADDRESS_REQUEST_FLAGS) $(DSKY_ADDRESS_REQUEST_CORE) $(DSKY_ADDRESS_REQUEST_TEST) -o $@
test-dsky-address-request: build/test-dsky-address-request
	build/test-dsky-address-request
sanitize-dsky-address-request:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_ADDRESS_REQUEST_FLAGS) $(DSKY_ADDRESS_REQUEST_CORE) $(DSKY_ADDRESS_REQUEST_TEST) -o build/test-dsky-address-request-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-address-request-sanitize
c64-dsky-address-request:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_ADDRESS_REQUEST_FLAGS) $(DSKY_ADDRESS_REQUEST_CORE) tests/dsky_address_request_check.c tools/dsky_address_request_diagnostic.c -Wl,-Map,build/dsky_address_request.map -o build/dsky_address_request.prg
differential-dsky-address-request:
	python3 tools/dsky_address_request_differential.py
	python3 tools/freeze_dsky_address_request.py
test: test-dsky-address-request
sanitize: sanitize-dsky-address-request
.PHONY: all test sanitize test-dsky-address-request sanitize-dsky-address-request c64-dsky-address-request differential-dsky-address-request
