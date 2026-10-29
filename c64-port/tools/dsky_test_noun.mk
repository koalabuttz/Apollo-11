# SPDX-License-Identifier: MIT
DSKY_TEST_NOUN_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
DSKY_TEST_NOUN_CORE = src/agc_word.c src/dsky_test_noun.c src/noun_reader.c src/fixed_data.c tests/dsky_test_noun_cases.c
all: build/test-dsky-test-noun build/dsky-test-noun-route-port
build/test-dsky-test-noun: tests/test_dsky_test_noun.c $(DSKY_TEST_NOUN_CORE) tests/dsky_test_noun_check.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_TEST_NOUN_FLAGS) $^ -o $@
build/dsky-test-noun-route-port: tools/dsky_test_noun_snapshot.c $(DSKY_TEST_NOUN_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DSKY_TEST_NOUN_FLAGS) $^ -o $@
test: build/test-dsky-test-noun
	build/test-dsky-test-noun
sanitize:
	python3 tools/dsky_test_noun_sanitize.py
