# SPDX-License-Identifier: MIT
EPHEMERIS_ADD_CORE = src/ephemeris_add.c src/agc_word.c src/agc_multiword.c tests/ephemeris_add_cases.c
all: build/ephemeris-add-integrated-stream build/test-ephemeris-add
build/ephemeris-add-integrated-stream: tools/ephemeris_add_snapshot.c $(EPHEMERIS_ADD_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
build/test-ephemeris-add: tests/test_ephemeris_add.c tests/ephemeris_add_target.c tests/frozen_words.c $(EPHEMERIS_ADD_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test: test-ephemeris-add-integrated
test-ephemeris-add-integrated: build/test-ephemeris-add
	./build/test-ephemeris-add
	python3 tools/ephemeris_add_host.py --cc "$(CC)"
sanitize: sanitize-ephemeris-add-integrated
sanitize-ephemeris-add-integrated: | build
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_ephemeris_add.c tests/ephemeris_add_target.c tests/frozen_words.c $(EPHEMERIS_ADD_CORE) -o build/test-ephemeris-add-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-ephemeris-add-integrated-san
	python3 tools/ephemeris_add_host.py --cc "$(CC)" --sanitize
.PHONY: test-ephemeris-add-integrated sanitize-ephemeris-add-integrated
