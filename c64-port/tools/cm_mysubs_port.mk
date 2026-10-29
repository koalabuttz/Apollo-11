# SPDX-License-Identifier: MIT
# Preserve qualified standalone recipes in a separate make scope.
CM_MYSUBS_PORT_CORE = src/cm_mysubs.c src/agc_word.c src/agc_multiword.c tests/cm_mysubs_cases.c
CM_MYSUBS_PORT_HEADERS = include/cm_mysubs.h tests/cm_mysubs_vectors.h
all: build/test-cm-mysubs
build/test-cm-mysubs: $(CM_MYSUBS_PORT_CORE) $(CM_MYSUBS_PORT_HEADERS) tests/test_cm_mysubs.c tests/frozen_words.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_cm_mysubs.c tests/frozen_words.c $(CM_MYSUBS_PORT_CORE) -o $@
test: test-cm-mysubs-port
test-cm-mysubs-port: build/test-cm-mysubs
	./build/test-cm-mysubs
sanitize: sanitize-cm-mysubs-port
sanitize-cm-mysubs-port: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined tests/test_cm_mysubs.c tests/frozen_words.c $(CM_MYSUBS_PORT_CORE) -o build/test-cm-mysubs-port-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-mysubs-port-sanitize
.PHONY: test-cm-mysubs-port sanitize-cm-mysubs-port
