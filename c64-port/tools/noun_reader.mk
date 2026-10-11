# SPDX-License-Identifier: MIT
NOUN_READER_CORE = src/agc_word.c src/fixed_data.c src/noun_reader.c tests/noun_reader_cases.c
NOUN_READER_HEADERS = include/noun_reader.h tests/noun_reader_vectors.h include/fixed_data.h include/fixed_data_symbols.h src/fixed_data_lm.inc src/fixed_data_cm.inc
all: build/test-noun-reader
build/test-noun-reader: $(NOUN_READER_CORE) $(NOUN_READER_HEADERS) tests/test_noun_reader.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(NOUN_READER_CORE) tests/test_noun_reader.c -o $@
test: test-noun-reader
test-noun-reader: build/test-noun-reader
	./build/test-noun-reader
sanitize: sanitize-noun-reader
sanitize-noun-reader: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(NOUN_READER_CORE) tests/test_noun_reader.c -o build/test-noun-reader-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-noun-reader-sanitize
.PHONY: test-noun-reader sanitize-noun-reader
