VECTOR_CORE = src/agc_word.c src/agc_multiword.c src/guidance_vector.c
VECTOR_INPUT = tests/prepare_cases.c tests/vector_cases.c
VECTOR_TEST = $(VECTOR_INPUT) tests/vector_check.c
VECTOR_HEADERS = $(PREPARE_HEADERS) include/guidance_vector.h tests/vector_cases.h tests/vector_vectors.h
VECTOR_DEFINITIONS = -DPREPARE_INPUT_ONLY
all: build/vector-snapshot build/test-vector
build/vector-snapshot: $(VECTOR_CORE) $(VECTOR_INPUT) platform/host/vector_snapshot.c $(VECTOR_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(VECTOR_DEFINITIONS) $(VECTOR_CORE) $(VECTOR_INPUT) platform/host/vector_snapshot.c -o $@
build/test-vector: $(VECTOR_CORE) $(VECTOR_TEST) tests/test_vector.c $(VECTOR_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(VECTOR_DEFINITIONS) $(VECTOR_CORE) $(VECTOR_TEST) tests/test_vector.c -o $@
test: test-vector
.PHONY: test-vector sanitize-vector c64-vector
test-vector: build/test-vector
	./build/test-vector
sanitize: sanitize-vector
sanitize-vector: | build
	$(CC) $(CPPFLAGS) $(VECTOR_DEFINITIONS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(VECTOR_CORE) $(VECTOR_TEST) tests/test_vector.c -o build/test-vector-sanitize
	./build/test-vector-sanitize
c64-vector: build/vector.prg
build/vector.prg: $(VECTOR_CORE) $(VECTOR_TEST) platform/c64/vector_diagnostic.c $(VECTOR_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) $(VECTOR_DEFINITIONS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/vector.map $(VECTOR_CORE) $(VECTOR_TEST) platform/c64/vector_diagnostic.c -o $@
