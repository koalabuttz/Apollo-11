# SPDX-License-Identifier: MIT
INTEGRATED_DSKY_SIGN_OUTPUT_CORE = src/agc_word.c src/dsky_pinball.c src/dsky_sign_output.c tests/dsky_sign_output_cases.c tests/dsky_sign_output_check.c
INTEGRATED_DSKY_SIGN_OUTPUT_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
all: build/test-dsky-sign-output-integrated build/dsky-sign-output-integrated-stream
build/test-dsky-sign-output-integrated: tests/test_dsky_sign_output.c $(INTEGRATED_DSKY_SIGN_OUTPUT_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_SIGN_OUTPUT_FLAGS) $^ -o $@
build/dsky-sign-output-integrated-stream: tools/dsky_sign_output_snapshot.c $(INTEGRATED_DSKY_SIGN_OUTPUT_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_SIGN_OUTPUT_FLAGS) $^ -o $@
test: test-dsky-sign-output-integrated
test-dsky-sign-output-integrated: build/test-dsky-sign-output-integrated build/dsky-sign-output-integrated-stream
	./build/test-dsky-sign-output-integrated
	python3 tools/qualified_stream_host.py --archive evidence/dsky-sign-output-reference.snapshots.xz --sha256 c6b38cdb46bc0a42b4dcf050b19b5b2122f56b3521e2cfdaaa0d546733191be6 --binary build/dsky-sign-output-integrated-stream
sanitize: sanitize-dsky-sign-output-integrated
sanitize-dsky-sign-output-integrated: | build
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_SIGN_OUTPUT_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_dsky_sign_output.c $(INTEGRATED_DSKY_SIGN_OUTPUT_CORE) -o build/test-dsky-sign-output-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-sign-output-integrated-san
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_SIGN_OUTPUT_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/dsky_sign_output_snapshot.c $(INTEGRATED_DSKY_SIGN_OUTPUT_CORE) -o build/dsky-sign-output-integrated-stream-san
	python3 tools/qualified_stream_host.py --archive evidence/dsky-sign-output-reference.snapshots.xz --sha256 c6b38cdb46bc0a42b4dcf050b19b5b2122f56b3521e2cfdaaa0d546733191be6 --binary build/dsky-sign-output-integrated-stream-san
.PHONY: test-dsky-sign-output-integrated sanitize-dsky-sign-output-integrated
