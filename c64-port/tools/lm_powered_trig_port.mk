# SPDX-License-Identifier: MIT
LM_POWERED_TRIG_PORT_CORE = src/lm_powered_trig.c src/guidance_trig.c src/guidance_product.c src/agc_word.c src/agc_multiword.c tests/lm_powered_trig_cases.c tests/frozen_words.c
all: build/test-lm-powered-trig-port
build/test-lm-powered-trig-port: $(LM_POWERED_TRIG_PORT_CORE) tests/test_lm_powered_trig.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_lm_powered_trig.c $(LM_POWERED_TRIG_PORT_CORE) -o $@
test: test-lm-powered-trig-port
test-lm-powered-trig-port: build/test-lm-powered-trig-port
	./build/test-lm-powered-trig-port
sanitize: sanitize-lm-powered-trig-port
sanitize-lm-powered-trig-port: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_lm_powered_trig.c $(LM_POWERED_TRIG_PORT_CORE) -o build/test-lm-powered-trig-port-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-powered-trig-port-san
.PHONY: test-lm-powered-trig-port sanitize-lm-powered-trig-port
