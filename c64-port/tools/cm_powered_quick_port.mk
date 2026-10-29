# SPDX-License-Identifier: MIT
CM_POWERED_QUICK_PORT_CORE = src/cm_powered_constants.c tests/test_cm_powered_constants.c src/cm_powered_quick_return.c src/cm_quictrig.c src/agc_word.c src/agc_multiword.c tests/cm_powered_quick_cases.c tests/cm_powered_trig_cases.c tests/frozen_words.c
all: build/test-cm-powered-quick-port
build/test-cm-powered-quick-port: $(CM_POWERED_QUICK_PORT_CORE) tests/test_cm_powered_quick.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_cm_powered_quick.c $(CM_POWERED_QUICK_PORT_CORE) -o $@
test: test-cm-powered-quick-port
test-cm-powered-quick-port: build/test-cm-powered-quick-port
	./build/test-cm-powered-quick-port
sanitize: sanitize-cm-powered-quick-port
sanitize-cm-powered-quick-port: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined tests/test_cm_powered_quick.c $(CM_POWERED_QUICK_PORT_CORE) -o build/test-cm-powered-quick-port-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-powered-quick-port-sanitize
.PHONY: test-cm-powered-quick-port sanitize-cm-powered-quick-port
