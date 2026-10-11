# SPDX-License-Identifier: MIT
CM_EPHEMERIS_VELOCITY_CORE = src/cm_ephemeris_velocity.c src/cm_ephemeris_time.c src/ephemeris_time_scale.c src/ephemeris_shift.c src/ephemeris_add.c src/interpreted_sign.c src/sband_scalar.c src/sband_vector_linear.c src/sband_shift.c src/guidance_shift.c src/agc_word.c src/agc_multiword.c tests/cm_ephemeris_time_cases.c
all: build/cm-ephemeris-velocity-integrated-stream build/test-cm-ephemeris-velocity
build/cm-ephemeris-velocity-integrated-stream: tools/cm_ephemeris_velocity_snapshot.c $(CM_EPHEMERIS_VELOCITY_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
build/test-cm-ephemeris-velocity: tests/test_cm_ephemeris_velocity.c tests/cm_ephemeris_velocity_target.c tests/frozen_words.c $(CM_EPHEMERIS_VELOCITY_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test: test-cm-ephemeris-velocity-integrated
test-cm-ephemeris-velocity-integrated: build/test-cm-ephemeris-velocity
	./build/test-cm-ephemeris-velocity
	python3 tools/cm_ephemeris_velocity_host.py --cc "$(CC)"
sanitize: sanitize-cm-ephemeris-velocity-integrated
sanitize-cm-ephemeris-velocity-integrated: | build
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_cm_ephemeris_velocity.c tests/cm_ephemeris_velocity_target.c tests/frozen_words.c $(CM_EPHEMERIS_VELOCITY_CORE) -o build/test-cm-ephemeris-velocity-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-ephemeris-velocity-integrated-san
	python3 tools/cm_ephemeris_velocity_host.py --cc "$(CC)" --sanitize
.PHONY: test-cm-ephemeris-velocity-integrated sanitize-cm-ephemeris-velocity-integrated
