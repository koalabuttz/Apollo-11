# SPDX-License-Identifier: MIT
SBAND_ORBIT_INTEGRATED_CORE = tests/sband_orbit_cases.c src/agc_word.c src/sband_orbit.c
SBAND_ORBIT_INTEGRATED_TEST = tests/test_sband_orbit.c tests/sband_orbit_target.c tests/frozen_words.c
build/sband-orbit-integrated-stream: tools/sband_orbit_snapshot.c $(SBAND_ORBIT_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
build/test-sband-orbit-integrated: $(SBAND_ORBIT_INTEGRATED_TEST) $(SBAND_ORBIT_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test-sband-orbit: build/sband-orbit-integrated-stream build/test-sband-orbit-integrated
	./build/test-sband-orbit-integrated
	python3 tools/sband_orbit_port_host.py --binary build/sband-orbit-integrated-stream
sanitize-sband-orbit: | build
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(SBAND_ORBIT_INTEGRATED_TEST) $(SBAND_ORBIT_INTEGRATED_CORE) -o build/test-sband-orbit-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-sband-orbit-integrated-san
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/sband_orbit_snapshot.c $(SBAND_ORBIT_INTEGRATED_CORE) -o build/sband-orbit-integrated-stream-san
	python3 tools/sband_orbit_port_host.py --binary build/sband-orbit-integrated-stream-san
.PHONY: test-sband-orbit sanitize-sband-orbit
