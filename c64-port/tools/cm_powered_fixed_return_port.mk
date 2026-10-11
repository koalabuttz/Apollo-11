# SPDX-License-Identifier: MIT
CM_POWERED_FIXED_RETURN_PORT_CORE = src/cm_powered_fixed_return.c src/agc_word.c tests/cm_powered_fixed_return_cases.c tests/cm_powered_interface_cases.c tests/cm_powered_axis_cases.c tests/cm_powered_trig_cases.c tests/frozen_words.c
all: build/test-cm-powered-fixed-return-port
build/test-cm-powered-fixed-return-port: $(CM_POWERED_FIXED_RETURN_PORT_CORE) tests/test_cm_powered_fixed_return.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_cm_powered_fixed_return.c $(CM_POWERED_FIXED_RETURN_PORT_CORE) -o $@
test: test-cm-powered-fixed-return-port
test-cm-powered-fixed-return-port: build/test-cm-powered-fixed-return-port
	./build/test-cm-powered-fixed-return-port
sanitize: sanitize-cm-powered-fixed-return-port
sanitize-cm-powered-fixed-return-port: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined tests/test_cm_powered_fixed_return.c $(CM_POWERED_FIXED_RETURN_PORT_CORE) -o build/test-cm-powered-fixed-return-port-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-powered-fixed-return-port-sanitize
.PHONY: test-cm-powered-fixed-return-port sanitize-cm-powered-fixed-return-port
