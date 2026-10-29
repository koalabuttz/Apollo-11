# SPDX-License-Identifier: MIT
SBAND_MOON_INTEGRATED_CORE = tests/sband_moon_cases.c src/agc_word.c src/sband_moon.c src/sband_vector_linear.c src/sband_shift.c src/guidance_shift.c src/agc_multiword.c
SBAND_MOON_INTEGRATED_TEST = tests/test_sband_moon.c tests/sband_moon_target.c tests/frozen_words.c
build/sband-moon-integrated-stream: tools/sband_moon_snapshot.c $(SBAND_MOON_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
build/test-sband-moon-integrated: $(SBAND_MOON_INTEGRATED_TEST) $(SBAND_MOON_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test-sband-moon: build/sband-moon-integrated-stream build/test-sband-moon-integrated
	./build/test-sband-moon-integrated
	python3 tools/sband_moon_port_host.py --binary build/sband-moon-integrated-stream
sanitize-sband-moon: | build
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(SBAND_MOON_INTEGRATED_TEST) $(SBAND_MOON_INTEGRATED_CORE) -o build/test-sband-moon-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-sband-moon-integrated-san
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/sband_moon_snapshot.c $(SBAND_MOON_INTEGRATED_CORE) -o build/sband-moon-integrated-stream-san
	python3 tools/sband_moon_port_host.py --binary build/sband-moon-integrated-stream-san
.PHONY: test-sband-moon sanitize-sband-moon
