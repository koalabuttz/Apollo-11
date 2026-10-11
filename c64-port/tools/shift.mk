# Native explicit scalar/vector shifts. Include from the port's Makefile.
SHIFT_DEFINITIONS = -DELAPSED_INPUT_ONLY -DPUSH_INPUT_ONLY -DSCALE_INPUT_ONLY -DPREPARE_INPUT_ONLY
SHIFT_CORE = src/agc_word.c src/agc_multiword.c src/guidance_shift.c
SHIFT_CASES = tests/scenarios.c tests/prepare_cases.c tests/scale_cases.c tests/push_cases.c tests/elapsed_cases.c tests/shift_cases.c
SHIFT_CHECK = tests/frozen_words.c tests/shift_check.c
SHIFT_HEADERS = include/guidance_shift.h include/guidance_elapsed.h include/guidance_scale.h include/guidance_prepare.h include/guidance_push.h include/guidance.h include/agc_word.h include/agc_multiword.h tests/shift_cases.h tests/shift_vectors.h tests/frozen_words.h tests/elapsed_cases.h tests/push_cases.h tests/scale_cases.h tests/prepare_cases.h tests/scenarios.h
build/test-shift: $(SHIFT_CORE) $(SHIFT_CASES) $(SHIFT_CHECK) tests/test_shift.c $(SHIFT_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SHIFT_DEFINITIONS) $(SHIFT_CORE) $(SHIFT_CASES) $(SHIFT_CHECK) tests/test_shift.c -o $@
build/shift-snapshot: $(SHIFT_CORE) $(SHIFT_CASES) tools/shift_snapshot.c $(SHIFT_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SHIFT_DEFINITIONS) $(SHIFT_CORE) $(SHIFT_CASES) tools/shift_snapshot.c -o $@
all: build/test-shift build/shift-snapshot
test: test-shift
.PHONY: test-shift sanitize-shift c64-shift
test-shift: build/test-shift
	./build/test-shift
sanitize-shift: build/shift-snapshot | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(SHIFT_DEFINITIONS) $(SHIFT_CORE) $(SHIFT_CASES) $(SHIFT_CHECK) tests/test_shift.c -o build/test-shift-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-shift-sanitize
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(SHIFT_DEFINITIONS) $(SHIFT_CORE) $(SHIFT_CASES) tools/shift_snapshot.c -o build/shift-snapshot-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/shift-snapshot-sanitize > build/shift-sanitize.snapshots
	./build/shift-snapshot > build/shift-native.snapshots
	cmp build/shift-sanitize.snapshots build/shift-native.snapshots
sanitize: sanitize-shift
c64-shift: build/shift.prg
build/shift.prg: $(SHIFT_CORE) $(SHIFT_CASES) $(SHIFT_CHECK) platform/c64/shift_diagnostic.c $(SHIFT_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) $(SHIFT_DEFINITIONS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/shift.map $(SHIFT_CORE) $(SHIFT_CASES) $(SHIFT_CHECK) platform/c64/shift_diagnostic.c -o $@
