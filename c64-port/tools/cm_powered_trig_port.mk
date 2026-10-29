# SPDX-License-Identifier: MIT
CM_POWERED_TRIG_PORT_CORE = src/cm_powered_trig.c src/guidance_trig.c src/guidance_product.c src/agc_word.c src/agc_multiword.c tests/cm_powered_trig_cases.c tests/frozen_words.c
all: build/test-cm-powered-trig-port
build/test-cm-powered-trig-port: $(CM_POWERED_TRIG_PORT_CORE) tests/test_cm_powered_trig.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_cm_powered_trig.c $(CM_POWERED_TRIG_PORT_CORE) -o $@
test: test-cm-powered-trig-port
test-cm-powered-trig-port: build/test-cm-powered-trig-port
	./build/test-cm-powered-trig-port
sanitize: sanitize-cm-powered-trig-port
sanitize-cm-powered-trig-port: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined tests/test_cm_powered_trig.c $(CM_POWERED_TRIG_PORT_CORE) -o build/test-cm-powered-trig-port-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-powered-trig-port-sanitize
.PHONY: test-cm-powered-trig-port sanitize-cm-powered-trig-port
