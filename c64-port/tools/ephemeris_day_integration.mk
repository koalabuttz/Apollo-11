# SPDX-License-Identifier: MIT
EPHEMERIS_DAY_CORE = src/ephemeris_day.c src/guidance_dp_divide.c src/agc_word.c src/agc_multiword.c tests/ephemeris_day_cases.c
all: build/ephemeris-day-integrated-stream build/test-ephemeris-day
build/ephemeris-day-integrated-stream: tools/ephemeris_day_snapshot.c $(EPHEMERIS_DAY_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
build/test-ephemeris-day: tests/test_ephemeris_day.c tests/ephemeris_day_target.c tests/frozen_words.c $(EPHEMERIS_DAY_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test: test-ephemeris-day-integrated
test-ephemeris-day-integrated: build/test-ephemeris-day
	./build/test-ephemeris-day
	python3 tools/ephemeris_day_host.py --cc "$(CC)"
sanitize: sanitize-ephemeris-day-integrated
sanitize-ephemeris-day-integrated: | build
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_ephemeris_day.c tests/ephemeris_day_target.c tests/frozen_words.c $(EPHEMERIS_DAY_CORE) -o build/test-ephemeris-day-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-ephemeris-day-integrated-san
	python3 tools/ephemeris_day_host.py --cc "$(CC)" --sanitize
.PHONY: test-ephemeris-day-integrated sanitize-ephemeris-day-integrated
