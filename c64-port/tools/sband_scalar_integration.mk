# SPDX-License-Identifier: MIT
SBAND_SCALAR_INTEGRATED_CORE = tests/sband_scalar_cases.c src/agc_word.c src/sband_scalar.c src/agc_multiword.c
SBAND_SCALAR_INTEGRATED_TEST = tests/test_sband_scalar.c tests/sband_scalar_target.c tests/frozen_words.c
build/sband-scalar-integrated-stream: tools/sband_scalar_snapshot.c $(SBAND_SCALAR_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
build/test-sband-scalar-integrated: $(SBAND_SCALAR_INTEGRATED_TEST) $(SBAND_SCALAR_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test-sband-scalar: build/sband-scalar-integrated-stream build/test-sband-scalar-integrated
	./build/test-sband-scalar-integrated
	python3 tools/sband_scalar_port_host.py --binary build/sband-scalar-integrated-stream
sanitize-sband-scalar: | build
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(SBAND_SCALAR_INTEGRATED_TEST) $(SBAND_SCALAR_INTEGRATED_CORE) -o build/test-sband-scalar-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-sband-scalar-integrated-san
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/sband_scalar_snapshot.c $(SBAND_SCALAR_INTEGRATED_CORE) -o build/sband-scalar-integrated-stream-san
	python3 tools/sband_scalar_port_host.py --binary build/sband-scalar-integrated-stream-san
.PHONY: test-sband-scalar sanitize-sband-scalar
