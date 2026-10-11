# SPDX-License-Identifier: MIT
CM_POWERED_AXIS_PORT_CORE = src/cm_powered_axis.c src/cm_rotation.c src/guidance_product.c src/agc_word.c src/agc_multiword.c tests/cm_powered_axis_cases.c tests/cm_powered_trig_cases.c tests/frozen_words.c
all: build/test-cm-powered-axis-port
build/test-cm-powered-axis-port: $(CM_POWERED_AXIS_PORT_CORE) tests/test_cm_powered_axis.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_cm_powered_axis.c $(CM_POWERED_AXIS_PORT_CORE) -o $@
test: test-cm-powered-axis-port
test-cm-powered-axis-port: build/test-cm-powered-axis-port
	./build/test-cm-powered-axis-port
sanitize: sanitize-cm-powered-axis-port
sanitize-cm-powered-axis-port: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined tests/test_cm_powered_axis.c $(CM_POWERED_AXIS_PORT_CORE) -o build/test-cm-powered-axis-port-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-powered-axis-port-sanitize
.PHONY: test-cm-powered-axis-port sanitize-cm-powered-axis-port
