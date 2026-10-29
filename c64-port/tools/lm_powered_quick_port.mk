# SPDX-License-Identifier: MIT
LM_POWERED_QUICK_PORT_CORE = src/lm_powered_constants.c tests/test_lm_powered_constants.c src/lm_powered_quick.c src/lm_single_trig.c src/agc_word.c src/agc_multiword.c tests/lm_powered_quick_cases.c tests/lm_powered_trig_cases.c tests/frozen_words.c
all: build/test-lm-powered-quick-port
build/test-lm-powered-quick-port: $(LM_POWERED_QUICK_PORT_CORE) tests/test_lm_powered_quick.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_lm_powered_quick.c $(LM_POWERED_QUICK_PORT_CORE) -o $@
test: test-lm-powered-quick-port
test-lm-powered-quick-port: build/test-lm-powered-quick-port
	./build/test-lm-powered-quick-port
sanitize: sanitize-lm-powered-quick-port
sanitize-lm-powered-quick-port: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_lm_powered_quick.c $(LM_POWERED_QUICK_PORT_CORE) -o build/test-lm-powered-quick-port-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-powered-quick-port-san
.PHONY: test-lm-powered-quick-port sanitize-lm-powered-quick-port
