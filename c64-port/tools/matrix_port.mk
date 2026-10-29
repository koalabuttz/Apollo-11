# SPDX-License-Identifier: MIT
include tools/matrix.mk
test: matrix-host
sanitize: sanitize-matrix-frozen
sanitize-matrix-frozen: | build
	$(CC) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(MATRIX_FLAGS) tests/test_matrix.c tests/matrix_check.c $(MATRIX_CORE) -o build/test-matrix-frozen-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-matrix-frozen-sanitize
.PHONY: sanitize-matrix-frozen
