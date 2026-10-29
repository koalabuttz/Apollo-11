# SPDX-License-Identifier: MIT
# Standalone milestone build; the parent owns integration into the main Makefile.
ACC_FLAGS = -std=c99 -Wall -Wextra -Werror -Iinclude -Itests -DUNIT_INPUT_ONLY -DDIVIDE_INPUT_ONLY -DSTORE_INPUT_ONLY -DELAPSED_INPUT_ONLY -DPUSH_INPUT_ONLY -DSCALE_INPUT_ONLY -DPREPARE_INPUT_ONLY
ACC_CORE = src/guidance_acceleration.c src/guidance_unit.c src/guidance_sum.c src/guidance_shift.c src/guidance_gravity.c src/guidance_norm.c src/guidance_dp_divide.c src/agc_word.c src/agc_multiword.c tests/scenarios.c tests/prepare_cases.c tests/scale_cases.c tests/push_cases.c tests/elapsed_cases.c tests/store_cases.c tests/divide_cases.c tests/unit_cases.c tests/acceleration_cases.c
ACC_TEST = tests/test_acceleration.c tests/frozen_words.c $(ACC_CORE)
.PHONY: acceleration-host acceleration-sanitize acceleration-c64
acceleration-host:
	gcc -O2 -Wconversion -pedantic $(ACC_FLAGS) $(ACC_TEST) -o build/test-acceleration
	build/test-acceleration
acceleration-sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(ACC_FLAGS) $(ACC_TEST) -o build/test-acceleration-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-acceleration-sanitize
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(ACC_FLAGS) tools/acceleration_snapshot.c $(ACC_CORE) -o build/acceleration-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/acceleration-native-sanitize > build/acceleration-sanitize.snapshots
	cmp build/acceleration-sanitize.snapshots build/acceleration-reference.snapshots
acceleration-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DACCELERATION_CHECK_ONLY $(ACC_FLAGS) tools/acceleration_diagnostic.c $(ACC_TEST) -Wl,-Map,build/acceleration.map -o build/acceleration.prg

all: build/acceleration-native build/test-acceleration
build/acceleration-native: $(ACC_CORE) tools/acceleration_snapshot.c tests/acceleration_cases.h include/guidance_acceleration.h | build
	$(CC) -O2 $(ACC_FLAGS) tools/acceleration_snapshot.c $(ACC_CORE) -o $@
build/test-acceleration: $(ACC_TEST) tests/acceleration_vectors.h tests/acceleration_cases.h include/guidance_acceleration.h | build
	$(CC) -O2 -Wconversion -pedantic $(ACC_FLAGS) $(ACC_TEST) -o $@
test: acceleration-host
sanitize: acceleration-sanitize
.PHONY: c64-acceleration
c64-acceleration: acceleration-c64
