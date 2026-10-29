# SPDX-License-Identifier: MIT
# Preserve qualified standalone recipes in a separate make scope.
LM_SINGLE_TRIG_PORT_CORE = src/lm_single_trig.c src/agc_word.c src/agc_multiword.c tests/lm_single_trig_cases.c
LM_SINGLE_TRIG_PORT_HEADERS = include/lm_single_trig.h tests/lm_single_trig_vectors.h
all: build/test-lm-single-trig
build/test-lm-single-trig: $(LM_SINGLE_TRIG_PORT_CORE) $(LM_SINGLE_TRIG_PORT_HEADERS) tests/test_lm_single_trig.c tests/frozen_words.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_lm_single_trig.c tests/frozen_words.c $(LM_SINGLE_TRIG_PORT_CORE) -o $@
test: test-lm-single-trig-port
test-lm-single-trig-port: build/test-lm-single-trig
	./build/test-lm-single-trig
sanitize: sanitize-lm-single-trig-port
sanitize-lm-single-trig-port: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined tests/test_lm_single_trig.c tests/frozen_words.c $(LM_SINGLE_TRIG_PORT_CORE) -o build/test-lm-single-trig-port-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-single-trig-port-sanitize
.PHONY: test-lm-single-trig-port sanitize-lm-single-trig-port
