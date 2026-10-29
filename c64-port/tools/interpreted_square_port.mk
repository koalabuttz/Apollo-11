# SPDX-License-Identifier: MIT
INTERPRETED_SQUARE_PORT_CORE = src/interpreted_square.c src/agc_word.c src/agc_multiword.c tests/interpreted_square_cases.c tests/frozen_words.c
all: build/test-interpreted-square-port
build/test-interpreted-square-port: $(INTERPRETED_SQUARE_PORT_CORE) tests/test_interpreted_square.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_interpreted_square.c $(INTERPRETED_SQUARE_PORT_CORE) -o $@
test: test-interpreted-square-port
test-interpreted-square-port: build/test-interpreted-square-port
	./build/test-interpreted-square-port
sanitize: sanitize-interpreted-square-port
sanitize-interpreted-square-port: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_interpreted_square.c $(INTERPRETED_SQUARE_PORT_CORE) -o build/test-interpreted-square-port-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-interpreted-square-port-san
.PHONY: test-interpreted-square-port sanitize-interpreted-square-port
