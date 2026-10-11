# SPDX-License-Identifier: MIT
DSKY_SIGN_OUTPUT_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
DSKY_SIGN_OUTPUT_CORE = src/agc_word.c src/dsky_pinball.c src/dsky_sign_output.c tests/dsky_sign_output_cases.c tests/dsky_sign_output_check.c
all: build/test-dsky-sign-output build/dsky-sign-output-route-port
build/test-dsky-sign-output: tests/test_dsky_sign_output.c $(DSKY_SIGN_OUTPUT_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_SIGN_OUTPUT_FLAGS) $^ -o $@
build/dsky-sign-output-route-port: tools/dsky_sign_output_snapshot.c $(DSKY_SIGN_OUTPUT_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_SIGN_OUTPUT_FLAGS) $^ -o $@
dsky-sign-output-host: build/test-dsky-sign-output
	build/test-dsky-sign-output
dsky-sign-output-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(DSKY_SIGN_OUTPUT_FLAGS) tests/test_dsky_sign_output.c $(DSKY_SIGN_OUTPUT_CORE) -o build/test-dsky-sign-output-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-sign-output-sanitize
	python3 tools/dsky_sign_output_sanitize.py
dsky-sign-output-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_SIGN_OUTPUT_FLAGS) tools/dsky_sign_output_diagnostic.c $(DSKY_SIGN_OUTPUT_CORE) -Wl,-Map,build/dsky_sign_output.map -o build/dsky_sign_output.prg
test: dsky-sign-output-host
sanitize: dsky-sign-output-sanitize
.PHONY: dsky-sign-output-host dsky-sign-output-sanitize dsky-sign-output-c64
