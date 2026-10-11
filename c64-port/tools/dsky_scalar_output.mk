# SPDX-License-Identifier: MIT
DSKY_SCALAR_OUTPUT_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
DSKY_SCALAR_OUTPUT_CORE = src/agc_word.c src/agc_multiword.c src/dsky_scalar_output.c tests/dsky_scalar_output_cases.c tests/dsky_scalar_output_check.c
all: build/test-dsky-scalar-output build/dsky-scalar-output-route-port
build/test-dsky-scalar-output: tests/test_dsky_scalar_output.c $(DSKY_SCALAR_OUTPUT_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_SCALAR_OUTPUT_FLAGS) $^ -o $@
build/dsky-scalar-output-route-port: tools/dsky_scalar_output_snapshot.c $(DSKY_SCALAR_OUTPUT_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_SCALAR_OUTPUT_FLAGS) $^ -o $@
dsky-scalar-output-host: build/test-dsky-scalar-output
	build/test-dsky-scalar-output
dsky-scalar-output-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(DSKY_SCALAR_OUTPUT_FLAGS) tests/test_dsky_scalar_output.c $(DSKY_SCALAR_OUTPUT_CORE) -o build/test-dsky-scalar-output-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-dsky-scalar-output-sanitize
	python3 tools/dsky_scalar_output_sanitize.py
dsky-scalar-output-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(DSKY_SCALAR_OUTPUT_FLAGS) tools/dsky_scalar_output_diagnostic.c $(DSKY_SCALAR_OUTPUT_CORE) -Wl,-Map,build/dsky_scalar_output.map -o build/dsky_scalar_output.prg
test: dsky-scalar-output-host
sanitize: dsky-scalar-output-sanitize
.PHONY: dsky-scalar-output-host dsky-scalar-output-sanitize dsky-scalar-output-c64
