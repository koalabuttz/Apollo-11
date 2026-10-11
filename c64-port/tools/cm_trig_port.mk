# SPDX-License-Identifier: MIT
# Preserve qualified standalone recipes in a separate make scope.
CM_TRIG_PORT_CORE = src/cm_quictrig.c src/agc_word.c src/agc_multiword.c tests/cm_trig_cases.c
CM_TRIG_PORT_HEADERS = include/cm_quictrig.h tests/cm_trig_vectors.h
all: build/test-cm-trig
build/test-cm-trig: $(CM_TRIG_PORT_CORE) $(CM_TRIG_PORT_HEADERS) tests/test_cm_trig.c tests/frozen_words.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_cm_trig.c tests/frozen_words.c $(CM_TRIG_PORT_CORE) -o $@
test: test-cm-trig-port
test-cm-trig-port: build/test-cm-trig
	./build/test-cm-trig
sanitize: sanitize-cm-trig-port
sanitize-cm-trig-port: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined tests/test_cm_trig.c tests/frozen_words.c $(CM_TRIG_PORT_CORE) -o build/test-cm-trig-port-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-trig-port-sanitize
.PHONY: test-cm-trig-port sanitize-cm-trig-port
