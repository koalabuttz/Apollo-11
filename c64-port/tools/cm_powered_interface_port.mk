# SPDX-License-Identifier: MIT
CM_POWERED_INTERFACE_PORT_CORE = src/cm_powered_interface.c src/cm_powered_axis.c src/cm_powered_trig.c src/guidance_trig.c src/cm_rotation.c src/guidance_product.c src/agc_word.c src/agc_multiword.c tests/cm_powered_interface_cases.c tests/cm_powered_axis_cases.c tests/cm_powered_trig_cases.c tests/frozen_words.c
all: build/test-cm-powered-interface-port
build/test-cm-powered-interface-port: $(CM_POWERED_INTERFACE_PORT_CORE) tests/test_cm_powered_interface.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_cm_powered_interface.c $(CM_POWERED_INTERFACE_PORT_CORE) -o $@
test: test-cm-powered-interface-port
test-cm-powered-interface-port: build/test-cm-powered-interface-port
	./build/test-cm-powered-interface-port
sanitize: sanitize-cm-powered-interface-port
sanitize-cm-powered-interface-port: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined tests/test_cm_powered_interface.c $(CM_POWERED_INTERFACE_PORT_CORE) -o build/test-cm-powered-interface-port-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-powered-interface-port-sanitize
.PHONY: test-cm-powered-interface-port sanitize-cm-powered-interface-port
