TRIG_DEFINITIONS = -DUNIT_INPUT_ONLY -DDIVIDE_INPUT_ONLY -DSTORE_INPUT_ONLY -DELAPSED_INPUT_ONLY -DPUSH_INPUT_ONLY -DSCALE_INPUT_ONLY -DPREPARE_INPUT_ONLY
TRIG_CORE = src/agc_word.c src/agc_multiword.c src/guidance_product.c src/guidance_trig.c
TRIG_INPUT = tests/prepare_cases.c tests/scale_cases.c tests/push_cases.c tests/elapsed_cases.c tests/store_cases.c tests/divide_cases.c tests/unit_cases.c tests/trig_cases.c
TRIG_CHECK = tests/trig_check.c tests/frozen_words.c
TRIG_HEADERS = include/guidance_trig.h include/guidance_norm.h include/guidance_product.h tests/trig_cases.h tests/trig_vectors.h
all: build/test-trig build/trig-native
test: test-trig
sanitize: sanitize-trig
.PHONY: test-trig sanitize-trig c64-trig
build/test-trig: $(TRIG_CORE) $(TRIG_INPUT) $(TRIG_CHECK) tests/test_trig.c $(TRIG_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TRIG_DEFINITIONS) $(TRIG_CORE) $(TRIG_INPUT) $(TRIG_CHECK) tests/test_trig.c -o $@
build/trig-native: $(TRIG_CORE) $(TRIG_INPUT) tools/trig_snapshot.c $(TRIG_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TRIG_DEFINITIONS) $(TRIG_CORE) $(TRIG_INPUT) tools/trig_snapshot.c -o $@
test-trig: build/test-trig
	./build/test-trig
sanitize-trig: | build
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(TRIG_DEFINITIONS) $(TRIG_CORE) $(TRIG_INPUT) $(TRIG_CHECK) tests/test_trig.c -o build/test-trig-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-trig-sanitize
	$(CC) $(CPPFLAGS) -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(TRIG_DEFINITIONS) $(TRIG_CORE) $(TRIG_INPUT) tools/trig_snapshot.c -o build/trig-snapshot-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/trig-snapshot-sanitize > build/trig-sanitize.snapshots
	cmp build/trig-native.snapshots build/trig-sanitize.snapshots
c64-trig: build/trig.prg
build/trig.prg: $(TRIG_CORE) $(TRIG_INPUT) $(TRIG_CHECK) platform/c64/trig_diagnostic.c $(TRIG_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) $(TRIG_DEFINITIONS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/trig.map $(TRIG_CORE) $(TRIG_INPUT) $(TRIG_CHECK) platform/c64/trig_diagnostic.c -o $@
