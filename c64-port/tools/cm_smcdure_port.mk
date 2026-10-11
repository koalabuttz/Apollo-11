# SPDX-License-Identifier: MIT
CM_SMCDURE_PORT_CORE = src/cm_smcdure.c src/guidance_product.c src/guidance_dp_divide.c src/guidance_sum.c src/guidance_shift.c src/agc_word.c src/agc_multiword.c tests/cm_smcdure_cases.c tests/cm_powered_interface_cases.c tests/cm_powered_axis_cases.c tests/cm_powered_trig_cases.c tests/frozen_words.c
all: build/test-cm-smcdure-port
build/test-cm-smcdure-port: $(CM_SMCDURE_PORT_CORE) tests/test_cm_smcdure.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_cm_smcdure.c $(CM_SMCDURE_PORT_CORE) -o $@
test: test-cm-smcdure-port
test-cm-smcdure-port: build/test-cm-smcdure-port
	./build/test-cm-smcdure-port
sanitize: sanitize-cm-smcdure-port
sanitize-cm-smcdure-port: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined tests/test_cm_smcdure.c $(CM_SMCDURE_PORT_CORE) -o build/test-cm-smcdure-port-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-smcdure-port-sanitize
.PHONY: test-cm-smcdure-port sanitize-cm-smcdure-port
