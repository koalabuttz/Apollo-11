# SPDX-License-Identifier: MIT
INVERSE_TRIG_SWEEP_PORT_CORE = src/inverse_trig.c src/guidance_norm.c src/guidance_product.c src/agc_word.c src/agc_multiword.c tests/inverse_trig_sweep_cases.c tests/frozen_words.c
all: build/test-inverse-trig-sweep-port
build/test-inverse-trig-sweep-port: $(INVERSE_TRIG_SWEEP_PORT_CORE) tests/test_inverse_trig_sweep.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_inverse_trig_sweep.c $(INVERSE_TRIG_SWEEP_PORT_CORE) -o $@
test: test-inverse-trig-sweep-port
test-inverse-trig-sweep-port: build/test-inverse-trig-sweep-port
	./build/test-inverse-trig-sweep-port
sanitize: sanitize-inverse-trig-sweep-port
sanitize-inverse-trig-sweep-port: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_inverse_trig_sweep.c $(INVERSE_TRIG_SWEEP_PORT_CORE) -o build/test-inverse-trig-sweep-port-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-inverse-trig-sweep-port-san
.PHONY: test-inverse-trig-sweep-port sanitize-inverse-trig-sweep-port
