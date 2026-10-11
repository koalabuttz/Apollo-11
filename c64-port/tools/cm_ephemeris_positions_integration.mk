# SPDX-License-Identifier: MIT
CM_EPHEMERIS_POSITIONS_CORE = src/guidance_product.c src/sband_trig.c src/guidance_trig.c src/sband_vector.c src/rtb_unit.c src/rtb_vector.c src/rtb_angles.c src/guidance_unit_phased.c src/guidance_length_phased.c src/guidance_norm.c src/cm_ephemeris_positions.c src/cm_ephemeris_solar.c src/cm_ephemeris_time.c src/ephemeris_time_scale.c src/ephemeris_shift.c src/ephemeris_add.c src/interpreted_sign.c src/sband_scalar.c src/sband_vector_linear.c src/sband_shift.c src/guidance_shift.c src/agc_word.c src/agc_multiword.c tests/cm_ephemeris_time_cases.c
all: build/cm-ephemeris-positions-integrated-stream build/test-cm-ephemeris-positions-guards
build/cm-ephemeris-positions-integrated-stream: tools/cm_ephemeris_positions_snapshot.c $(CM_EPHEMERIS_POSITIONS_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
build/test-cm-ephemeris-positions-guards: tests/test_cm_ephemeris_positions_guards.c $(CM_EPHEMERIS_POSITIONS_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test: test-cm-ephemeris-positions-integrated
test-cm-ephemeris-positions-integrated: build/test-cm-ephemeris-positions-guards
	./build/test-cm-ephemeris-positions-guards
	python3 tools/cm_ephemeris_positions_host.py --cc "$(CC)"
	python3 tools/cm_ephemeris_positions_phases_host.py --cc "$(CC)"
sanitize: sanitize-cm-ephemeris-positions-integrated
sanitize-cm-ephemeris-positions-integrated: | build
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_cm_ephemeris_positions_guards.c $(CM_EPHEMERIS_POSITIONS_CORE) -o build/test-cm-ephemeris-positions-guards-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-cm-ephemeris-positions-guards-san
	python3 tools/cm_ephemeris_positions_host.py --cc "$(CC)" --sanitize
	python3 tools/cm_ephemeris_positions_phases_host.py --cc "$(CC)" --sanitize
.PHONY: test-cm-ephemeris-positions-integrated sanitize-cm-ephemeris-positions-integrated
