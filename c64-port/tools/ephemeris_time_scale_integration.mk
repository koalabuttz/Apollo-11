# SPDX-License-Identifier: MIT
EPHEMERIS_TIME_SCALE_CORE = src/ephemeris_time_scale.c src/agc_word.c src/agc_multiword.c tests/ephemeris_time_scale_cases.c
all: build/ephemeris-time-scale-integrated-stream build/test-ephemeris-time-scale
build/ephemeris-time-scale-integrated-stream: tools/ephemeris_time_scale_snapshot.c $(EPHEMERIS_TIME_SCALE_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
build/test-ephemeris-time-scale: tests/test_ephemeris_time_scale.c tests/ephemeris_time_scale_target.c tests/frozen_words.c $(EPHEMERIS_TIME_SCALE_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test: test-ephemeris-time-scale-integrated
test-ephemeris-time-scale-integrated: build/test-ephemeris-time-scale
	./build/test-ephemeris-time-scale
	python3 tools/ephemeris_time_scale_host.py --cc "$(CC)"
sanitize: sanitize-ephemeris-time-scale-integrated
sanitize-ephemeris-time-scale-integrated: | build
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_ephemeris_time_scale.c tests/ephemeris_time_scale_target.c tests/frozen_words.c $(EPHEMERIS_TIME_SCALE_CORE) -o build/test-ephemeris-time-scale-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-ephemeris-time-scale-integrated-san
	python3 tools/ephemeris_time_scale_host.py --cc "$(CC)" --sanitize
.PHONY: test-ephemeris-time-scale-integrated sanitize-ephemeris-time-scale-integrated
