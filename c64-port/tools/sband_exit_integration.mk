# SPDX-License-Identifier: MIT
SBAND_EXIT_INTEGRATED_CORE = tests/sband_exit_cases.c src/agc_word.c src/sband_exit.c
SBAND_EXIT_INTEGRATED_TEST = tests/test_sband_exit.c tests/sband_exit_target.c tests/frozen_words.c
build/sband-exit-integrated-stream: tools/sband_exit_snapshot.c $(SBAND_EXIT_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
build/test-sband-exit-integrated: $(SBAND_EXIT_INTEGRATED_TEST) $(SBAND_EXIT_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test-sband-exit: build/sband-exit-integrated-stream build/test-sband-exit-integrated
	./build/test-sband-exit-integrated
	python3 tools/sband_exit_port_host.py --binary build/sband-exit-integrated-stream
sanitize-sband-exit: | build
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(SBAND_EXIT_INTEGRATED_TEST) $(SBAND_EXIT_INTEGRATED_CORE) -o build/test-sband-exit-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-sband-exit-integrated-san
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/sband_exit_snapshot.c $(SBAND_EXIT_INTEGRATED_CORE) -o build/sband-exit-integrated-stream-san
	python3 tools/sband_exit_port_host.py --binary build/sband-exit-integrated-stream-san
.PHONY: test-sband-exit sanitize-sband-exit
