# SPDX-License-Identifier: MIT
# Preserve qualified standalone recipes in a separate make scope.
CM_SINGLE_RETURN_PORT_CORE = src/cm_single_return.c src/agc_word.c tests/cm_single_return_cases.c tests/cm_trig_cases.c src/cm_quictrig.c src/agc_multiword.c
CM_SINGLE_RETURN_PORT_HEADERS = include/cm_single_return.h tests/cm_single_return_vectors.h
all: build/test-cm-single-return
build/test-cm-single-return: $(CM_SINGLE_RETURN_PORT_CORE) $(CM_SINGLE_RETURN_PORT_HEADERS) tests/test_cm_single_return.c tests/frozen_words.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -DCM_SINGLE_RETURN_COMPLETE tests/test_cm_single_return.c tests/frozen_words.c $(CM_SINGLE_RETURN_PORT_CORE) -o $@
test: test-cm-single-return-port
test-cm-single-return-port: build/test-cm-single-return
	./build/test-cm-single-return
sanitize: sanitize-cm-single-return-port
sanitize-cm-single-return-port: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined -DCM_SINGLE_RETURN_COMPLETE tests/test_cm_single_return.c tests/frozen_words.c $(CM_SINGLE_RETURN_PORT_CORE) -o build/test-cm-single-return-port-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-single-return-port-sanitize
.PHONY: test-cm-single-return-port sanitize-cm-single-return-port
