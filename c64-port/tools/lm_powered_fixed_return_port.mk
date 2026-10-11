# SPDX-License-Identifier: MIT
LM_POWERED_FIXED_RETURN_PORT_CORE = src/lm_powered_fixed_return.c src/agc_word.c tests/lm_powered_fixed_return_cases.c tests/lm_powered_interface_cases.c tests/lm_powered_axis_cases.c tests/lm_powered_trig_cases.c tests/frozen_words.c
all: build/test-lm-powered-fixed-return-port
build/test-lm-powered-fixed-return-port: $(LM_POWERED_FIXED_RETURN_PORT_CORE) tests/test_lm_powered_fixed_return.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_lm_powered_fixed_return.c $(LM_POWERED_FIXED_RETURN_PORT_CORE) -o $@
test: test-lm-powered-fixed-return-port
test-lm-powered-fixed-return-port: build/test-lm-powered-fixed-return-port
	./build/test-lm-powered-fixed-return-port
sanitize: sanitize-lm-powered-fixed-return-port
sanitize-lm-powered-fixed-return-port: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_lm_powered_fixed_return.c $(LM_POWERED_FIXED_RETURN_PORT_CORE) -o build/test-lm-powered-fixed-return-port-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-powered-fixed-return-port-san
.PHONY: test-lm-powered-fixed-return-port sanitize-lm-powered-fixed-return-port
