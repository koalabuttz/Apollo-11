# Native general scalar sums and vector reverse subtraction.
SUM_DEFINITIONS = -DVECTOR_INPUT_ONLY -DPREPARE_INPUT_ONLY
SUM_CORE = src/agc_word.c src/agc_multiword.c src/guidance_sum.c
SUM_CASES = tests/prepare_cases.c tests/vector_cases.c tests/sum_cases.c
SUM_CHECK = tests/frozen_words.c tests/sum_check.c
SUM_HEADERS = include/guidance_sum.h include/guidance_vector.h include/guidance_elapsed.h include/guidance_scale.h include/guidance_prepare.h include/guidance.h include/agc_word.h include/agc_multiword.h tests/sum_cases.h tests/sum_vectors.h tests/vector_cases.h tests/prepare_cases.h tests/frozen_words.h
build/test-sum: $(SUM_CORE) $(SUM_CASES) $(SUM_CHECK) tests/test_sum.c $(SUM_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SUM_DEFINITIONS) $(SUM_CORE) $(SUM_CASES) $(SUM_CHECK) tests/test_sum.c -o $@
build/sum-snapshot: $(SUM_CORE) $(SUM_CASES) tools/sum_snapshot.c $(SUM_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SUM_DEFINITIONS) $(SUM_CORE) $(SUM_CASES) tools/sum_snapshot.c -o $@
all: build/test-sum build/sum-snapshot
test: test-sum
.PHONY: test-sum sanitize-sum c64-sum
test-sum: build/test-sum
	./build/test-sum
sanitize-sum: build/sum-snapshot | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(SUM_DEFINITIONS) $(SUM_CORE) $(SUM_CASES) $(SUM_CHECK) tests/test_sum.c -o build/test-sum-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-sum-sanitize
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(SUM_DEFINITIONS) $(SUM_CORE) $(SUM_CASES) tools/sum_snapshot.c -o build/sum-snapshot-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/sum-snapshot-sanitize > build/sum-sanitize.snapshots
	./build/sum-snapshot > build/sum-native.snapshots
	cmp build/sum-sanitize.snapshots build/sum-native.snapshots
sanitize: sanitize-sum
c64-sum: build/sum.prg
build/sum.prg: $(SUM_CORE) $(SUM_CASES) $(SUM_CHECK) platform/c64/sum_diagnostic.c $(SUM_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) $(SUM_DEFINITIONS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/sum.map $(SUM_CORE) $(SUM_CASES) $(SUM_CHECK) platform/c64/sum_diagnostic.c -o $@
