# SPDX-License-Identifier: MIT
LM_FLESHPOT_PORT_CORE = src/lm_fleshpot.c src/guidance_product.c src/agc_word.c src/agc_multiword.c tests/lm_fleshpot_cases.c tests/lm_powered_axis_cases.c tests/lm_powered_trig_cases.c tests/frozen_words.c
all: build/test-lm-fleshpot-port
build/test-lm-fleshpot-port: $(LM_FLESHPOT_PORT_CORE) tests/test_lm_fleshpot.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_lm_fleshpot.c $(LM_FLESHPOT_PORT_CORE) -o $@
test: test-lm-fleshpot-port
test-lm-fleshpot-port: build/test-lm-fleshpot-port
	./build/test-lm-fleshpot-port
sanitize: sanitize-lm-fleshpot-port
sanitize-lm-fleshpot-port: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_lm_fleshpot.c $(LM_FLESHPOT_PORT_CORE) -o build/test-lm-fleshpot-port-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-fleshpot-port-san
.PHONY: test-lm-fleshpot-port sanitize-lm-fleshpot-port
