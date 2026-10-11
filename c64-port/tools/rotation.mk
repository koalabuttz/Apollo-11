# SPDX-License-Identifier: MIT
# Standalone milestone build; the parent owns integration into the main Makefile.
ROT_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests -DUNIT_INPUT_ONLY -DDIVIDE_INPUT_ONLY -DSTORE_INPUT_ONLY -DELAPSED_INPUT_ONLY -DPUSH_INPUT_ONLY -DSCALE_INPUT_ONLY -DPREPARE_INPUT_ONLY
ROT_CORE = src/guidance_rotation.c src/guidance_product.c src/agc_word.c src/agc_multiword.c tests/scenarios.c tests/prepare_cases.c tests/scale_cases.c tests/push_cases.c tests/elapsed_cases.c tests/store_cases.c tests/divide_cases.c tests/unit_cases.c tests/rotation_cases.c
ROT_TEST = tests/test_rotation.c tests/frozen_words.c $(ROT_CORE)
.PHONY: rotation-host rotation-sanitize rotation-c64
rotation-host:
	gcc -O2 -Wconversion -pedantic $(ROT_FLAGS) $(ROT_TEST) -o build/test-rotation
	build/test-rotation
rotation-sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(ROT_FLAGS) $(ROT_TEST) -o build/test-rotation-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-rotation-sanitize
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(ROT_FLAGS) tools/rotation_snapshot.c $(ROT_CORE) -o build/rotation-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/rotation-native-sanitize > build/rotation-sanitize.snapshots
	cmp build/rotation-sanitize.snapshots build/rotation-reference.snapshots
rotation-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DROTATION_CHECK_ONLY $(ROT_FLAGS) tools/rotation_diagnostic.c $(ROT_TEST) -Wl,-Map,build/rotation.map -o build/rotation.prg

all: build/rotation-native build/test-rotation
build/rotation-native: $(ROT_CORE) tools/rotation_snapshot.c tests/rotation_cases.h include/guidance_rotation.h | build
	$(CC) -O2 $(ROT_FLAGS) tools/rotation_snapshot.c $(ROT_CORE) -o $@
build/test-rotation: $(ROT_TEST) tests/rotation_vectors.h tests/rotation_cases.h include/guidance_rotation.h | build
	$(CC) -O2 -Wconversion -pedantic $(ROT_FLAGS) $(ROT_TEST) -o $@
test: rotation-host
sanitize: rotation-sanitize
.PHONY: c64-rotation
c64-rotation: rotation-c64
