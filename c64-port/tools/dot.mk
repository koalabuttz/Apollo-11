# Native unrounded vector DOT. Include from the port's Makefile.
DOT_DEFINITIONS = -DELAPSED_INPUT_ONLY -DPUSH_INPUT_ONLY -DSCALE_INPUT_ONLY -DPREPARE_INPUT_ONLY
DOT_CORE = src/agc_word.c src/agc_multiword.c src/guidance_product.c src/guidance_dot.c
DOT_CASES = tests/scenarios.c tests/prepare_cases.c tests/scale_cases.c tests/push_cases.c tests/elapsed_cases.c tests/dot_cases.c
DOT_CHECK = tests/frozen_words.c tests/dot_check.c
DOT_HEADERS = include/guidance_dot.h include/guidance_product.h include/guidance_divide.h include/guidance_elapsed.h include/guidance_scale.h include/guidance_prepare.h include/guidance_push.h include/guidance.h include/agc_word.h include/agc_multiword.h tests/dot_cases.h tests/dot_vectors.h tests/frozen_words.h tests/elapsed_cases.h tests/push_cases.h tests/scale_cases.h tests/prepare_cases.h tests/scenarios.h
build/test-dot: $(DOT_CORE) $(DOT_CASES) $(DOT_CHECK) tests/test_dot.c $(DOT_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DOT_DEFINITIONS) $(DOT_CORE) $(DOT_CASES) $(DOT_CHECK) tests/test_dot.c -o $@
build/dot-snapshot: $(DOT_CORE) $(DOT_CASES) tools/dot_snapshot.c $(DOT_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DOT_DEFINITIONS) $(DOT_CORE) $(DOT_CASES) tools/dot_snapshot.c -o $@
all: build/test-dot build/dot-snapshot
test: test-dot
.PHONY: test-dot sanitize-dot c64-dot
test-dot: build/test-dot
	./build/test-dot
sanitize-dot: build/dot-snapshot | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(DOT_DEFINITIONS) $(DOT_CORE) $(DOT_CASES) $(DOT_CHECK) tests/test_dot.c -o build/test-dot-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dot-sanitize
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(DOT_DEFINITIONS) $(DOT_CORE) $(DOT_CASES) tools/dot_snapshot.c -o build/dot-snapshot-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/dot-snapshot-sanitize > build/dot-sanitize.snapshots
	./build/dot-snapshot > build/dot-native.snapshots
	cmp build/dot-sanitize.snapshots build/dot-native.snapshots
sanitize: sanitize-dot
c64-dot: build/dot.prg
build/dot.prg: $(DOT_CORE) $(DOT_CASES) $(DOT_CHECK) platform/c64/dot_diagnostic.c $(DOT_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) $(DOT_DEFINITIONS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/dot.map $(DOT_CORE) $(DOT_CASES) $(DOT_CHECK) platform/c64/dot_diagnostic.c -o $@
