# SPDX-License-Identifier: MIT
DSKY_LOAD_REQUEST_FLAGS = -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Iinclude -Itests
DSKY_LOAD_REQUEST_CORE = src/agc_word.c src/agc_multiword.c src/dsky_pinball.c src/dsky_octal_display.c src/dsky_machine_cadr.c src/dsky_five_blank.c src/fixed_data.c src/noun_reader.c src/dsky_address_request.c src/dsky_nv_update_general.c src/dsky_noun_check.c src/dsky_load_request.c tests/p66_bias_cases.c tests/dsky_load_request_cases.c
DSKY_LOAD_REQUEST_TEST = tests/dsky_load_request_check.c tests/test_dsky_load_request.c
all: build/test-dsky-load-request
build/test-dsky-load-request: $(DSKY_LOAD_REQUEST_CORE) $(DSKY_LOAD_REQUEST_TEST) tests/dsky_load_request_vectors.h
	$(CC) -O2 -Wconversion -pedantic $(DSKY_LOAD_REQUEST_FLAGS) $(DSKY_LOAD_REQUEST_CORE) $(DSKY_LOAD_REQUEST_TEST) -o $@
test-dsky-load-request: build/test-dsky-load-request
	build/test-dsky-load-request
sanitize-dsky-load-request:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(DSKY_LOAD_REQUEST_FLAGS) $(DSKY_LOAD_REQUEST_CORE) $(DSKY_LOAD_REQUEST_TEST) -o build/test-dsky-load-request-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-load-request-sanitize
c64-dsky-load-request:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_LOAD_REQUEST_TARGET_LM $(DSKY_LOAD_REQUEST_FLAGS) $(DSKY_LOAD_REQUEST_CORE) tests/dsky_load_request_check.c tools/dsky_load_request_diagnostic.c -Wl,-Map,build/dsky_load_request-lm.map -o build/dsky_load_request-lm.prg
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DDSKY_LOAD_REQUEST_TARGET_CM $(DSKY_LOAD_REQUEST_FLAGS) $(DSKY_LOAD_REQUEST_CORE) tests/dsky_load_request_check.c tools/dsky_load_request_diagnostic.c -Wl,-Map,build/dsky_load_request-cm.map -o build/dsky_load_request-cm.prg
differential-dsky-load-request:
	python3 tools/dsky_load_request_differential.py
	python3 tools/freeze_dsky_load_request.py
test: test-dsky-load-request
sanitize: sanitize-dsky-load-request
.PHONY: all test sanitize test-dsky-load-request sanitize-dsky-load-request c64-dsky-load-request differential-dsky-load-request
