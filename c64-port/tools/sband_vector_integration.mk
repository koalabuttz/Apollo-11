# SPDX-License-Identifier: MIT
SBAND_VECTOR_INTEGRATED_CORE = src/sband_vector.c src/agc_word.c src/agc_multiword.c tests/sband_vector_cases.c
SBAND_VECTOR_INTEGRATED_TEST = tests/test_sband_vector.c tests/sband_vector_target.c tests/frozen_words.c
all: build/sband-vector-integrated-stream build/test-sband-vector-integrated
build/sband-vector-integrated-stream: tools/sband_vector_snapshot.c $(SBAND_VECTOR_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
build/test-sband-vector-integrated: $(SBAND_VECTOR_INTEGRATED_TEST) $(SBAND_VECTOR_INTEGRATED_CORE) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test: test-sband-vector-integrated
test-sband-vector-integrated: build/sband-vector-integrated-stream build/test-sband-vector-integrated
	./build/test-sband-vector-integrated
	python3 tools/sband_vector_port_host.py --binary build/sband-vector-integrated-stream
sanitize: sanitize-sband-vector-integrated
sanitize-sband-vector-integrated: | build
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(SBAND_VECTOR_INTEGRATED_TEST) $(SBAND_VECTOR_INTEGRATED_CORE) -o build/test-sband-vector-integrated-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-sband-vector-integrated-san
	$(CC) $(CPPFLAGS) -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tools/sband_vector_snapshot.c $(SBAND_VECTOR_INTEGRATED_CORE) -o build/sband-vector-integrated-stream-san
	python3 tools/sband_vector_port_host.py --binary build/sband-vector-integrated-stream-san
.PHONY: test-sband-vector-integrated sanitize-sband-vector-integrated
