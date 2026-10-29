# SPDX-License-Identifier: MIT
SBAND_LENGTH_INTEGRATED_CORE = tests/sband_length_cases.c src/agc_word.c src/sband_length.c src/guidance_length_phased.c src/guidance_norm.c src/agc_multiword.c
SBAND_LENGTH_INTEGRATED_TEST = tests/test_sband_length.c tests/sband_length_target.c tests/frozen_words.c
build/sband-length-integrated-stream: tools/sband_length_snapshot.c $(SBAND_LENGTH_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
build/test-sband-length-integrated: $(SBAND_LENGTH_INTEGRATED_TEST) $(SBAND_LENGTH_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test-sband-length: build/sband-length-integrated-stream build/test-sband-length-integrated
	./build/test-sband-length-integrated
	python3 tools/sband_length_port_host.py --binary build/sband-length-integrated-stream
sanitize-sband-length: | build
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(SBAND_LENGTH_INTEGRATED_TEST) $(SBAND_LENGTH_INTEGRATED_CORE) -o build/test-sband-length-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-sband-length-integrated-san
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/sband_length_snapshot.c $(SBAND_LENGTH_INTEGRATED_CORE) -o build/sband-length-integrated-stream-san
	python3 tools/sband_length_port_host.py --binary build/sband-length-integrated-stream-san
.PHONY: test-sband-length sanitize-sband-length
