# SPDX-License-Identifier: MIT
SBAND_LINEAR_INTEGRATED_CORE = tests/sband_linear_cases.c src/agc_word.c src/sband_vector_linear.c src/agc_multiword.c
SBAND_LINEAR_INTEGRATED_TEST = tests/test_sband_linear.c tests/sband_linear_target.c tests/frozen_words.c
build/sband-linear-integrated-stream: tools/sband_linear_snapshot.c $(SBAND_LINEAR_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
build/test-sband-linear-integrated: $(SBAND_LINEAR_INTEGRATED_TEST) $(SBAND_LINEAR_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test-sband-linear: build/sband-linear-integrated-stream build/test-sband-linear-integrated
	./build/test-sband-linear-integrated
	python3 tools/sband_linear_port_host.py --binary build/sband-linear-integrated-stream
sanitize-sband-linear: | build
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(SBAND_LINEAR_INTEGRATED_TEST) $(SBAND_LINEAR_INTEGRATED_CORE) -o build/test-sband-linear-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-sband-linear-integrated-san
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/sband_linear_snapshot.c $(SBAND_LINEAR_INTEGRATED_CORE) -o build/sband-linear-integrated-stream-san
	python3 tools/sband_linear_port_host.py --binary build/sband-linear-integrated-stream-san
.PHONY: test-sband-linear sanitize-sband-linear
