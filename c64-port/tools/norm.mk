NORM_CORE = src/agc_word.c src/agc_multiword.c src/guidance_norm.c
NORM_HEADERS = include/guidance_norm.h tests/norm_cases.h tests/norm_vectors.h tests/frozen_words.h
all: build/norm-native build/test-norm
build/norm-native: $(NORM_CORE) tests/norm_snapshot.c $(NORM_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(NORM_CORE) tests/norm_snapshot.c -o $@
build/test-norm: $(NORM_CORE) tests/test_norm.c tests/frozen_words.c $(NORM_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(NORM_CORE) tests/test_norm.c tests/frozen_words.c -o $@
test: test-norm
.PHONY: test-norm sanitize-norm c64-norm
test-norm: build/test-norm
	./build/test-norm
sanitize: sanitize-norm
sanitize-norm: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(NORM_CORE) tests/test_norm.c tests/frozen_words.c -o build/test-norm-sanitize
	./build/test-norm-sanitize
c64-norm: build/norm.prg
build/norm.prg: $(NORM_CORE) tests/test_norm.c tests/frozen_words.c tools/norm_diagnostic.c $(NORM_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -DNORM_CHECK_ONLY -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/norm.map $(NORM_CORE) tests/test_norm.c tests/frozen_words.c tools/norm_diagnostic.c -o $@
