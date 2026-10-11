# SPDX-License-Identifier: MIT
SBAND_DOT_INTEGRATED_CORE = tests/sband_dot_cases.c src/agc_word.c src/sband_dot.c src/agc_multiword.c
SBAND_DOT_INTEGRATED_TEST = tests/test_sband_dot.c tests/sband_dot_target.c tests/frozen_words.c
build/sband-dot-integrated-stream: tools/sband_dot_snapshot.c $(SBAND_DOT_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
build/test-sband-dot-integrated: $(SBAND_DOT_INTEGRATED_TEST) $(SBAND_DOT_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test-sband-dot: build/sband-dot-integrated-stream build/test-sband-dot-integrated
	./build/test-sband-dot-integrated
	python3 tools/sband_dot_port_host.py --binary build/sband-dot-integrated-stream
sanitize-sband-dot: | build
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(SBAND_DOT_INTEGRATED_TEST) $(SBAND_DOT_INTEGRATED_CORE) -o build/test-sband-dot-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-sband-dot-integrated-san
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/sband_dot_snapshot.c $(SBAND_DOT_INTEGRATED_CORE) -o build/sband-dot-integrated-stream-san
	python3 tools/sband_dot_port_host.py --binary build/sband-dot-integrated-stream-san
.PHONY: test-sband-dot sanitize-sband-dot
