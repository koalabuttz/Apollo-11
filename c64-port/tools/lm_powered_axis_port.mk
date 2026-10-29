# SPDX-License-Identifier: MIT
LM_POWERED_AXIS_PORT_CORE = src/lm_powered_axis.c src/guidance_rotation.c src/guidance_product.c src/agc_word.c src/agc_multiword.c tests/lm_powered_axis_cases.c tests/lm_powered_trig_cases.c tests/frozen_words.c
all: build/test-lm-powered-axis-port
build/test-lm-powered-axis-port: $(LM_POWERED_AXIS_PORT_CORE) tests/test_lm_powered_axis.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_lm_powered_axis.c $(LM_POWERED_AXIS_PORT_CORE) -o $@
test: test-lm-powered-axis-port
test-lm-powered-axis-port: build/test-lm-powered-axis-port
	./build/test-lm-powered-axis-port
sanitize: sanitize-lm-powered-axis-port
sanitize-lm-powered-axis-port: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_lm_powered_axis.c $(LM_POWERED_AXIS_PORT_CORE) -o build/test-lm-powered-axis-port-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-powered-axis-port-san
.PHONY: test-lm-powered-axis-port sanitize-lm-powered-axis-port
