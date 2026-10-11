# SPDX-License-Identifier: MIT
CM_EPHEMERIS_TIME_CORE = src/cm_ephemeris_time.c src/ephemeris_time_scale.c src/ephemeris_shift.c src/ephemeris_add.c src/interpreted_sign.c src/agc_word.c src/agc_multiword.c tests/cm_ephemeris_time_cases.c
all: build/cm-ephemeris-time-integrated-stream build/test-cm-ephemeris-time
build/cm-ephemeris-time-integrated-stream: tools/cm_ephemeris_time_snapshot.c $(CM_EPHEMERIS_TIME_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
build/test-cm-ephemeris-time: tests/test_cm_ephemeris_time.c tests/cm_ephemeris_time_target.c tests/frozen_words.c $(CM_EPHEMERIS_TIME_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test: test-cm-ephemeris-time-integrated
test-cm-ephemeris-time-integrated: build/test-cm-ephemeris-time
	./build/test-cm-ephemeris-time
	python3 tools/cm_ephemeris_time_host.py --cc "$(CC)"
sanitize: sanitize-cm-ephemeris-time-integrated
sanitize-cm-ephemeris-time-integrated: | build
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_cm_ephemeris_time.c tests/cm_ephemeris_time_target.c tests/frozen_words.c $(CM_EPHEMERIS_TIME_CORE) -o build/test-cm-ephemeris-time-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-ephemeris-time-integrated-san
	python3 tools/cm_ephemeris_time_host.py --cc "$(CC)" --sanitize
.PHONY: test-cm-ephemeris-time-integrated sanitize-cm-ephemeris-time-integrated
