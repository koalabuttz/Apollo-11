# SPDX-License-Identifier: MIT
INTEGRATED_DSKY_SCALAR_OUTPUT_CORE = src/agc_word.c src/agc_multiword.c src/dsky_scalar_output.c tests/dsky_scalar_output_cases.c tests/dsky_scalar_output_check.c
INTEGRATED_DSKY_SCALAR_OUTPUT_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
all: build/test-dsky-scalar-output-integrated build/dsky-scalar-output-integrated-stream
build/test-dsky-scalar-output-integrated: tests/test_dsky_scalar_output.c $(INTEGRATED_DSKY_SCALAR_OUTPUT_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_SCALAR_OUTPUT_FLAGS) $^ -o $@
build/dsky-scalar-output-integrated-stream: tools/dsky_scalar_output_snapshot.c $(INTEGRATED_DSKY_SCALAR_OUTPUT_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_SCALAR_OUTPUT_FLAGS) $^ -o $@
test: test-dsky-scalar-output-integrated
test-dsky-scalar-output-integrated: build/test-dsky-scalar-output-integrated build/dsky-scalar-output-integrated-stream
	./build/test-dsky-scalar-output-integrated
	python3 tools/qualified_stream_host.py --archive evidence/dsky-scalar-output-reference.snapshots.xz --sha256 883201fba7dd146a5cf1bd8534c9e1f866d4decfd194353200e04327595b00fa --binary build/dsky-scalar-output-integrated-stream
sanitize: sanitize-dsky-scalar-output-integrated
sanitize-dsky-scalar-output-integrated: | build
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_SCALAR_OUTPUT_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_dsky_scalar_output.c $(INTEGRATED_DSKY_SCALAR_OUTPUT_CORE) -o build/test-dsky-scalar-output-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-scalar-output-integrated-san
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_SCALAR_OUTPUT_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/dsky_scalar_output_snapshot.c $(INTEGRATED_DSKY_SCALAR_OUTPUT_CORE) -o build/dsky-scalar-output-integrated-stream-san
	python3 tools/qualified_stream_host.py --archive evidence/dsky-scalar-output-reference.snapshots.xz --sha256 883201fba7dd146a5cf1bd8534c9e1f866d4decfd194353200e04327595b00fa --binary build/dsky-scalar-output-integrated-stream-san
.PHONY: test-dsky-scalar-output-integrated sanitize-dsky-scalar-output-integrated
