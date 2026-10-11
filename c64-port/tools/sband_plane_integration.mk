# SPDX-License-Identifier: MIT
SBAND_PLANE_INTEGRATED_CORE = tests/sband_plane_cases.c src/agc_word.c src/sband_plane.c src/sband_vector.c src/sband_shift.c src/guidance_shift.c src/rtb_unit.c src/rtb_vector.c src/guidance_unit_phased.c src/guidance_length_phased.c src/guidance_norm.c src/agc_multiword.c
SBAND_PLANE_INTEGRATED_TEST = tests/test_sband_plane.c tests/sband_plane_target.c tests/frozen_words.c
build/sband-plane-integrated-stream: tools/sband_plane_snapshot.c $(SBAND_PLANE_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
build/test-sband-plane-integrated: $(SBAND_PLANE_INTEGRATED_TEST) $(SBAND_PLANE_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test-sband-plane: build/sband-plane-integrated-stream build/test-sband-plane-integrated
	./build/test-sband-plane-integrated
	python3 tools/sband_plane_port_host.py --binary build/sband-plane-integrated-stream
sanitize-sband-plane: | build
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(SBAND_PLANE_INTEGRATED_TEST) $(SBAND_PLANE_INTEGRATED_CORE) -o build/test-sband-plane-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-sband-plane-integrated-san
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/sband_plane_snapshot.c $(SBAND_PLANE_INTEGRATED_CORE) -o build/sband-plane-integrated-stream-san
	python3 tools/sband_plane_port_host.py --binary build/sband-plane-integrated-stream-san
.PHONY: test-sband-plane sanitize-sband-plane
