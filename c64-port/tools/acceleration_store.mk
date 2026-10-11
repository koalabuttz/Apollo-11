# SPDX-License-Identifier: MIT
# Standalone milestone build; the parent owns integration into the main Makefile.
AS_FLAGS = -DACCELERATION_INPUT_ONLY -std=c99 -Wall -Wextra -Werror -Iinclude -Itests -DUNIT_INPUT_ONLY -DDIVIDE_INPUT_ONLY -DSTORE_INPUT_ONLY -DELAPSED_INPUT_ONLY -DPUSH_INPUT_ONLY -DSCALE_INPUT_ONLY -DPREPARE_INPUT_ONLY
AS_CORE = src/guidance_acceleration_store.c src/guidance_acceleration_phased.c src/guidance_unit.c src/guidance_sum.c src/guidance_shift.c src/guidance_gravity.c src/guidance_norm.c src/guidance_dp_divide.c src/agc_word.c src/agc_multiword.c tests/scenarios.c tests/prepare_cases.c tests/scale_cases.c tests/push_cases.c tests/elapsed_cases.c tests/store_cases.c tests/divide_cases.c tests/unit_cases.c tests/acceleration_cases.c tests/acceleration_phased_cases.c
AS_TEST = tests/test_acceleration_store.c tests/frozen_words.c $(AS_CORE)
.PHONY: acceleration_store-host acceleration_store-sanitize acceleration_store-c64
acceleration_store-host:
	gcc -O2 -Wconversion -pedantic $(AS_FLAGS) $(AS_TEST) -o build/test-acceleration-store
	build/test-acceleration-store
acceleration_store-sanitize:
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(AS_FLAGS) $(AS_TEST) -o build/test-acceleration-store-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/test-acceleration-store-sanitize
	gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer $(AS_FLAGS) tools/acceleration_store_snapshot.c $(AS_CORE) -o build/acceleration-store-native-sanitize
	ASAN_OPTIONS=detect_leaks=0 build/acceleration-store-native-sanitize > build/acceleration-store-sanitize.snapshots
	cmp build/acceleration-store-sanitize.snapshots build/acceleration-store-reference.snapshots
acceleration_store-c64:
	$(MOS_CC) -Os -fno-static-stack -mlto-zp=0 -DACCELERATION_STORE_CHECK_ONLY -DACCELERATION_STORE_SELECTED=$(if $(PHASE),$(PHASE),0) $(AS_FLAGS) tools/acceleration_store_diagnostic.c $(AS_TEST) -Wl,-Map,build/acceleration-store$(PHASE).map -o build/acceleration-store$(PHASE).prg

all: build/acceleration-store-native build/test-acceleration-store
build/acceleration-store-native: $(AS_CORE) tools/acceleration_store_snapshot.c tests/acceleration_cases.h include/guidance_acceleration_store.h | build
	$(CC) -O2 $(AS_FLAGS) tools/acceleration_store_snapshot.c $(AS_CORE) -o $@
build/test-acceleration-store: $(AS_TEST) tests/acceleration_store_vectors.h tests/acceleration_cases.h include/guidance_acceleration_store.h | build
	$(CC) -O2 -Wconversion -pedantic $(AS_FLAGS) $(AS_TEST) -o $@
test: acceleration_store-host
sanitize: acceleration_store-sanitize
.PHONY: c64-acceleration-store
c64-acceleration-store: acceleration_store-c64

.PHONY: acceleration_store-differential
acceleration_store-differential:
	python3 tools/acceleration_store_differential.py
