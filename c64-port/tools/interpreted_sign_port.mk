# SPDX-License-Identifier: MIT
INTERPRETED_SIGN_PORT_CORE = src/interpreted_sign.c src/agc_word.c src/agc_multiword.c tests/interpreted_sign_cases.c tests/frozen_words.c
all: build/test-interpreted-sign-port
build/test-interpreted-sign-port: $(INTERPRETED_SIGN_PORT_CORE) tests/test_interpreted_sign.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_interpreted_sign.c $(INTERPRETED_SIGN_PORT_CORE) -o $@
test: test-interpreted-sign-port
test-interpreted-sign-port: build/test-interpreted-sign-port
	./build/test-interpreted-sign-port
sanitize: sanitize-interpreted-sign-port
sanitize-interpreted-sign-port: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_interpreted_sign.c $(INTERPRETED_SIGN_PORT_CORE) -o build/test-interpreted-sign-port-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-interpreted-sign-port-san
.PHONY: test-interpreted-sign-port sanitize-interpreted-sign-port
