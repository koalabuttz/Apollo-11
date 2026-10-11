# SPDX-License-Identifier: MIT
CM_RCS_MATRIX_PORT_CORE = src/cm_rcs_matrix.c src/cm_single_return.c src/cm_quictrig.c src/agc_word.c src/agc_multiword.c tests/cm_rcs_matrix_cases.c tests/frozen_words.c
all: build/test-cm-rcs-matrix-port
build/test-cm-rcs-matrix-port: $(CM_RCS_MATRIX_PORT_CORE) tests/test_cm_rcs_matrix.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_cm_rcs_matrix.c $(CM_RCS_MATRIX_PORT_CORE) -o $@
test: test-cm-rcs-matrix-port
test-cm-rcs-matrix-port: build/test-cm-rcs-matrix-port
	./build/test-cm-rcs-matrix-port
sanitize: sanitize-cm-rcs-matrix-port
sanitize-cm-rcs-matrix-port: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_cm_rcs_matrix.c $(CM_RCS_MATRIX_PORT_CORE) -o build/test-cm-rcs-matrix-port-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-rcs-matrix-port-san
.PHONY: test-cm-rcs-matrix-port sanitize-cm-rcs-matrix-port
