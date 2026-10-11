# SPDX-License-Identifier: MIT
MATRIX_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
MATRIX_CORE = src/agc_word.c src/agc_multiword.c src/guidance_product.c src/matrix_native.c tests/matrix_cases.c
matrix-host:
	$(CC) -O2 $(MATRIX_FLAGS) tests/test_matrix.c tests/matrix_check.c $(MATRIX_CORE) -o build/test-matrix
	build/test-matrix
matrix-sanitize:
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(MATRIX_FLAGS) tests/test_matrix.c tests/matrix_check.c $(MATRIX_CORE) -o build/test-matrix-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-matrix-sanitize
	python3 tools/matrix_sanitize_stream.py
matrix-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 $(MATRIX_FLAGS) tools/matrix_diagnostic.c tests/matrix_check.c $(MATRIX_CORE) -Wl,-Map,build/matrix.map -o build/matrix.prg
matrix-differential:
	python3 tools/matrix_differential.py
all: build/matrix-native
build/matrix-native: tools/matrix_snapshot.c $(MATRIX_CORE) | build
	$(CC) -O2 $(MATRIX_FLAGS) $^ -o $@
.PHONY: matrix-host matrix-sanitize matrix-c64 matrix-differential
