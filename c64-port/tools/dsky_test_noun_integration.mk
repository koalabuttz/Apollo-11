# SPDX-License-Identifier: MIT
INTEGRATED_DSKY_TEST_NOUN_CORE = src/agc_word.c src/dsky_test_noun.c src/noun_reader.c src/fixed_data.c tests/dsky_test_noun_cases.c
INTEGRATED_DSKY_TEST_NOUN_FLAGS = -ffunction-sections -Wl,--gc-sections -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests
all: build/test-dsky-test-noun-integrated build/dsky-test-noun-integrated-stream
build/test-dsky-test-noun-integrated: tests/test_dsky_test_noun.c $(INTEGRATED_DSKY_TEST_NOUN_CORE) tests/dsky_test_noun_check.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_TEST_NOUN_FLAGS) $^ -o $@
build/dsky-test-noun-integrated-stream: tools/dsky_test_noun_snapshot.c $(INTEGRATED_DSKY_TEST_NOUN_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INTEGRATED_DSKY_TEST_NOUN_FLAGS) $^ -o $@
test: test-dsky-test-noun-integrated
test-dsky-test-noun-integrated: build/test-dsky-test-noun-integrated build/dsky-test-noun-integrated-stream
	./build/test-dsky-test-noun-integrated
	python3 tools/qualified_stream_host.py --archive evidence/dsky-test-noun-reference.snapshots.xz --sha256 aad2039802f35066b224e47777dea37b6054d892a98103679bcd530f28b3d069 --binary build/dsky-test-noun-integrated-stream
sanitize: sanitize-dsky-test-noun-integrated
sanitize-dsky-test-noun-integrated: | build
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_TEST_NOUN_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_dsky_test_noun.c $(INTEGRATED_DSKY_TEST_NOUN_CORE) tests/dsky_test_noun_check.c -o build/test-dsky-test-noun-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dsky-test-noun-integrated-san
	$(CC) $(CPPFLAGS) $(INTEGRATED_DSKY_TEST_NOUN_FLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/dsky_test_noun_snapshot.c $(INTEGRATED_DSKY_TEST_NOUN_CORE) -o build/dsky-test-noun-integrated-stream-san
	python3 tools/qualified_stream_host.py --archive evidence/dsky-test-noun-reference.snapshots.xz --sha256 aad2039802f35066b224e47777dea37b6054d892a98103679bcd530f28b3d069 --binary build/dsky-test-noun-integrated-stream-san
.PHONY: test-dsky-test-noun-integrated sanitize-dsky-test-noun-integrated
