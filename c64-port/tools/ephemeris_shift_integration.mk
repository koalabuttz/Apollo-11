# SPDX-License-Identifier: MIT
EPHEMERIS_SHIFT_CORE = src/ephemeris_shift.c src/agc_word.c src/agc_multiword.c tests/ephemeris_shift_cases.c
all: build/ephemeris-shift-integrated-stream build/test-ephemeris-shift
build/ephemeris-shift-integrated-stream: tools/ephemeris_shift_snapshot.c $(EPHEMERIS_SHIFT_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
build/test-ephemeris-shift: tests/test_ephemeris_shift.c tests/ephemeris_shift_target.c tests/frozen_words.c $(EPHEMERIS_SHIFT_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test: test-ephemeris-shift-integrated
test-ephemeris-shift-integrated: build/test-ephemeris-shift
	./build/test-ephemeris-shift
	python3 tools/ephemeris_shift_host.py --cc "$(CC)"
sanitize: sanitize-ephemeris-shift-integrated
sanitize-ephemeris-shift-integrated: | build
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_ephemeris_shift.c tests/ephemeris_shift_target.c tests/frozen_words.c $(EPHEMERIS_SHIFT_CORE) -o build/test-ephemeris-shift-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-ephemeris-shift-integrated-san
	python3 tools/ephemeris_shift_host.py --cc "$(CC)" --sanitize
.PHONY: test-ephemeris-shift-integrated sanitize-ephemeris-shift-integrated
