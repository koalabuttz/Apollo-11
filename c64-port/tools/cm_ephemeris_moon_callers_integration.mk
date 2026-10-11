# SPDX-License-Identifier: MIT
CM_EPHEMERIS_MOON_CALLERS_CORE = src/cm_ephemeris_moon_callers.c src/cm_ephemeris_time.c src/ephemeris_time_scale.c src/ephemeris_shift.c src/ephemeris_add.c src/interpreted_sign.c src/sband_vector_linear.c src/sband_shift.c src/guidance_shift.c src/agc_word.c src/agc_multiword.c tests/cm_ephemeris_time_cases.c
all: build/cm-ephemeris-moon-callers-integrated-stream build/test-cm-ephemeris-moon-callers
build/cm-ephemeris-moon-callers-integrated-stream: tools/cm_ephemeris_moon_callers_snapshot.c $(CM_EPHEMERIS_MOON_CALLERS_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
build/test-cm-ephemeris-moon-callers: tests/test_cm_ephemeris_moon_callers.c tests/cm_ephemeris_moon_callers_target.c tests/frozen_words.c $(CM_EPHEMERIS_MOON_CALLERS_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test: test-cm-ephemeris-moon-callers-integrated
test-cm-ephemeris-moon-callers-integrated: build/test-cm-ephemeris-moon-callers
	./build/test-cm-ephemeris-moon-callers
	python3 tools/cm_ephemeris_moon_callers_host.py --cc "$(CC)"
sanitize: sanitize-cm-ephemeris-moon-callers-integrated
sanitize-cm-ephemeris-moon-callers-integrated: | build
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_cm_ephemeris_moon_callers.c tests/cm_ephemeris_moon_callers_target.c tests/frozen_words.c $(CM_EPHEMERIS_MOON_CALLERS_CORE) -o build/test-cm-ephemeris-moon-callers-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-ephemeris-moon-callers-integrated-san
	python3 tools/cm_ephemeris_moon_callers_host.py --cc "$(CC)" --sanitize
.PHONY: test-cm-ephemeris-moon-callers-integrated sanitize-cm-ephemeris-moon-callers-integrated
